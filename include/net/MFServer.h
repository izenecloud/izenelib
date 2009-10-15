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


template<	
		typename ServiceHandle,
		typename MapType =boost::unordered_map<std::string, ServiceItem<ServiceHandle> > > class MFServer {

public:
	MFServer(unsigned int nThreads, unsigned int serverPort,
			const std::string& controllerIP, unsigned int controllerPort
			);
	bool createServiceList(MapType& serviceList) {
		serviceList_ = serviceList;
		if( !registerServiceList() )
		 		return false;  
		return createThreadObjectPool();
			
	}
	void setServiceHandle(const boost::shared_ptr<ServiceHandle> &serviceHandle){
		serviceHandle_ = serviceHandle;		
	}
	
	//must be called before createServiceList
	void setAgentInfo(const string& agentInfo){
		messageServer_->setAgentInfo(agentInfo);
	}
	
	bool run(void);
private:

	bool registerServiceList(void);

	bool createThreadObjectPool();

private:
	///@brief   The number of threads to handle the services
	unsigned int nThreads_;

	///@brief   The thread pool for the ID services
	wiselib::thread_pool::ThreadObjectPool threadObjPool_;

	/// @brief  Holds the information related to the controller
	MessageFrameworkNode controllerInfo_;

	///@brief   Handler to the services
	boost::shared_ptr<ServiceHandle> serviceHandle_;

	///@brief   The information list of related services 
	MapType serviceList_;

	///@brief MessageServerFull or MessageServerLight
	MessageServerPtr messageServer_;
};

/////////////////////////////////////////////////////////////////////////////////////////

template< typename ServiceHandle, typename MapType> MFServer<
 ServiceHandle, MapType>::MFServer(unsigned int nThreads,
		unsigned int serverPort, const std::string& controllerIP,
		unsigned int controllerPort):nThreads_(nThreads),
controllerInfo_(controllerIP, controllerPort)
{
	string serverName = std::string(typeid(MessageServer).name() ) ;
	
	messageServer_.reset( new MessageServer(
			MF_SERVER_ARG (serverName, serverPort, controllerInfo_) )  );
}

template< typename ServiceHandle, typename MapType > bool MFServer<
		ServiceHandle, MapType>::run(void) {
    bool ret = false;
    boost::system_time wakeupTime;
    wiselib::thread_pool::ThreadObject* threadObject = NULL;
    WorkerThread<ServiceHandle>* workerObject = NULL;
    try
    { 
    	assert(serviceHandle_ != NULL);
        // main loop    	
        vector< ServiceRequestInfoPtr > requests;
        LOG(INFO) << " Starting service..." << endl;       
        while( true )
        {        
        	// receive service request
            //TODO: Method is blocked: should we measure mf time?  @by MyungHyun - 2009-02-04
            messageServer_->getServiceRequestList(requests);   
            if( requests.size() >= 1 )
            {
                // serve all of requests
                vector< ServiceRequestInfoPtr >::iterator iter = requests.begin();          
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
                            LOG(INFO) << "No idle threadObject. Waiting for a moment." << std::endl;
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

template<  typename ServiceHandle, typename MapType > bool MFServer<
		 ServiceHandle, MapType>::createThreadObjectPool() {

	for (unsigned int i = 0; i < nThreads_; i++) {
		wiselib::thread_pool::ThreadObject* threadObject = NULL;

		threadObject = new WorkerThread<ServiceHandle>();
		if ( !threadObject)
			return false;

		threadObject->setThreadObjectPool( &threadObjPool_);
		threadObject->createThread( true);
		threadObjPool_.add(threadObject);

	} // end - for

	return true;
}

template<  typename ServiceHandle, typename MapType > bool MFServer<
		 ServiceHandle, MapType>::registerServiceList() {
	assert(messageServer_->getAgentInfo() != "" );
	typename MapType::iterator iter = serviceList_.begin();
			for ( ; iter != serviceList_.end(); iter++ )
			{
				int tryCount = 10;					
				while ( ! messageServer_->registerService( ServiceInfo(iter->first) )  )				       
				{						
					LOG(ERROR)<< "Register Service : service = " << iter->first
					<< ", tryCount = " << tryCount<<". Waiting for controller ....." << std::endl;	
					usleep ( 10000 ); // 10ms
					tryCount--;					
				} // end - while()

				if ( tryCount == 0 )
					return false;

			} // end - for		
			return true;

}

}
#endif
