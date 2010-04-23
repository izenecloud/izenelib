/**
 * @brief   Defines the MFServer class
 * @author  Peisheng Wang
 * @date    08-17-2009
 * @details
 *
 */

#ifndef _MFSERVER_H_
#define _MFSERVER_H_

#include <net/message_framework.h>
#include <net/ServiceHandleBase.h>
#include <net/ServiceItem.h>
#include <net/WorkerThread.h>

#include <wiselib/thread-pool/ThreadObjectPool.h>

#include <boost/smart_ptr.hpp>
#include <boost/unordered_map.hpp>

namespace messageframework{

template
<
    typename ServiceHandle,
    typename MapType = boost::unordered_map<std::string,
                                            ServiceItem<ServiceHandle> >
>
class MFServer
{
public:
    typedef ServiceHandle service_handle_type;
    typedef MapType map_type;

    MFServer(unsigned int serverPort,
             const std::string& controllerIP,
             unsigned int controllerPort,
             unsigned int nThreads);

    void setServiceHandle(const boost::shared_ptr<ServiceHandle>& serviceHandle)
    {
        BOOST_ASSERT(serviceHandle);
        serviceHandle_ = serviceHandle;
    }

    const boost::shared_ptr<ServiceHandle>& serviceHandle() const
    {
        return serviceHandle_;
    }

    /// @warn must be called before addService
    void setAgentInfo(const std::string& agentInfo)
    {
        BOOST_ASSERT(messageServer_);
        messageServer_->setAgentInfo(agentInfo);
    }

    /// @brief adds service
    /// @return \c false if failed to register
    /// @pre non empty agent info has been set using setAgentInfo
    bool addService(const std::string& name,
                    const ServiceItem<ServiceHandle>& callback);

    bool run();

private:
    bool registerService_(const std::string& name,
                          const ServiceItem<ServiceHandle>& callback);

    bool createThreadObjectPool_();

private:
    ///@brief MessageServerFull or MessageServerLight
    MessageServerPtr messageServer_;

    /// @brief Handler to the services
    boost::shared_ptr<ServiceHandle> serviceHandle_;

    /// @brief The information list of related services
    MapType serviceList_;

    ///@brief The number of threads to handle the services
    unsigned int nThreads_;

    ///@brief   The thread pool for the ID services
    wiselib::thread_pool::ThreadObjectPool threadObjPool_;
};

////////////////////////////////////////////////////////////////////////////////

template<typename ServiceHandle, typename MapType>
MFServer<ServiceHandle, MapType>::MFServer(
    unsigned int serverPort,
    const std::string& controllerIP,
    unsigned int controllerPort,
    unsigned int nThreads
)
: messageServer_()
, serviceHandle_()
, serviceList_()
, nThreads_(nThreads)
, threadObjPool_()
{
    std::string serverName(typeid(MessageServer).name());
    MessageFrameworkNode controllerInfo(controllerIP, controllerPort);
    messageServer_.reset(
        new MessageServer(MF_SERVER_ARG(serverName, serverPort, controllerInfo))
    );
}

template<typename ServiceHandle, typename MapType>
bool MFServer<ServiceHandle, MapType>::addService(
    const std::string& name,
    const ServiceItem<ServiceHandle>& callback
)
{
    if (!registerService_(name, callback))
    {
        serviceList_.erase(name);
        return false;
    }

    serviceList_[name] = callback;
    return true;
}

template<typename ServiceHandle, typename MapType>
bool MFServer<ServiceHandle, MapType>::run(void)
{
    if (! (messageServer_
           && serviceHandle_))
    {
        // not ready
        DLOG(ERROR) << "Data not ready for MFServer" << endl;
        return false;
    }

    if (! createThreadObjectPool_())
    {
        DLOG(ERROR) << "Failed to init thread pool for MFServer" << endl;
        return false;
    }

    bool ret = false;
    boost::system_time wakeupTime;
    wiselib::thread_pool::ThreadObject* threadObject = NULL;
    WorkerThread<ServiceHandle>* workerObject = NULL;
    try
    {
        // main loop
        std::vector< ServiceRequestInfoPtr > requests;
        DLOG(INFO) << " Starting service..." << endl;
        while( true )
        {
            // receive service request
            //TODO: Method is blocked: should we measure mf time?  @by MyungHyun - 2009-02-04
            messageServer_->getServiceRequestList(requests);
            if( requests.size() >= 1 )
            {
                // serve all of requests
                std::vector< ServiceRequestInfoPtr >::iterator iter = requests.begin();
                while( iter != requests.end() )
                {
                    // find service in registered service
                    //  cout << "Processing request : "<< (*iter)->getServiceName() << endl;
                    typename MapType::iterator service =
                        serviceList_.find( (*iter)->getServiceName() );
                    if( service != serviceList_.end() )
                    {
                        while( false == (ret = threadObjPool_.get(threadObject)) )
                        {
                            DLOG(INFO) << "No idle threadObject. Waiting for a moment." << std::endl;
                            wakeupTime = boost::get_system_time() + boost::posix_time::milliseconds(5);
                            boost::thread::sleep( wakeupTime );
                        }

                        workerObject = static_cast<WorkerThread< ServiceHandle >* >(threadObject);
                        workerObject->setMessageServer( messageServer_);
                        workerObject->setRequestInfo(*iter);
                        workerObject->setServiceHandler( serviceHandle_ );
                        workerObject->setServiceItem(service->second);
                        workerObject->run();
                    }
                    else
                    {
                        serviceHandle_->no_callback(*iter);
                    }
                    // next request
                    iter++;
                }

            }
            //STOP_PROFILER(proIndexProcess)
        }
    }
    catch( ... )
    {
        threadObjPool_.shutdownAllThread();
        return false;
    }

    threadObjPool_.shutdownAllThread();
    return true;
}

template<typename ServiceHandle, typename MapType>
bool MFServer<ServiceHandle, MapType>::createThreadObjectPool_()
{
    for (unsigned int i = 0; i < nThreads_; i++)
    {
        wiselib::thread_pool::ThreadObject* threadObject
            = new WorkerThread<ServiceHandle>();

        if (!threadObject)
        {
            threadObjPool_.shutdownAllThread();
            return false;
        }

        threadObject->setThreadObjectPool(&threadObjPool_);
        threadObject->createThread(true);
        threadObjPool_.add(threadObject);
        //only one thread for cobra restrict version   
        COBRA_RESTRICT_BREAK
    } // end - for

    return true;
}

template<typename ServiceHandle, typename MapType>
bool MFServer<ServiceHandle, MapType>::registerService_(
    const std::string& name,
    const ServiceItem<ServiceHandle>& callback
)
{
    static const int tryLimit = 10;

    int tryCount = 0;
    ServiceInfo service(name);
    while (!messageServer_->registerService(service))
    {
        ++tryCount;
        DLOG(ERROR) << "Register Service : service = " << name
                   << ", failed = " << tryCount
                   << ". Waiting for controller ....." << std::endl;

        if (tryCount >= tryLimit)
        {
            return false;
        }

        usleep(100000); // 100ms
    }

    return true;
}

}
#endif
