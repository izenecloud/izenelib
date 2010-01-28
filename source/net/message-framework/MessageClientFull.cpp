///
///  @file : MessageClientFull.cpp
///  @date : 8/5/2008
///  @author : TuanQuang Nguyen
///

/**************** Include module header files *******************/
#include <net/message-framework/MessageClientFull.h>
#include <net/message-framework/MessageFrameworkConfiguration.h>
#include <net/message-framework/ServiceMessage.h>
#include <net/message-framework/MessageType.h>
#include <net/message-framework/ClientIdRequestMessage.h>

/**************** Include boost header files *******************/
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

/**************** Include std header files *******************/
#include <iostream>
#include <sstream>

using namespace std;

namespace messageframework
{

    /***************************************************************************
     Description: construct a MessageClientFull given the client name and
     information of controller. The MessageClientFull will connects to controller.
     Input:
     ioservice - main io service
     clientName - name of client
     controller - information of controller
     ***************************************************************************/
    MessageClientFull::MessageClientFull(const std::string& clientName,
                                         const MessageFrameworkNode& controllerInfo) :
            messageDispatcher_(this, this, this), asyncConnector_(this, io_service_)
    {
        ownerManager_ = clientName;
        //  information of service that retrieves permission of service
        servicePermisionServer_ = controllerInfo;

        // timeout is 1 second
        timeOutMilliSecond_ = TIME_OUT_IN_MILISECOND;

        controllerNode_.nodeIP_ = getHostIp(io_service_, controllerInfo.nodeIP_);
        controllerNode_.nodePort_ = controllerInfo.nodePort_;
        controllerNode_.nodeName_ = controllerInfo.nodeName_;

        connectionToControllerEstablished_ = false;
        asyncConnector_.connect(controllerInfo.nodeIP_, controllerInfo.nodePort_);

        // create thread for I/O operations
        ioThread_ = new boost::thread(boost::bind(&boost::asio::io_service::run, &io_service_));

        // waiting until connection to controller is established
        boost::mutex::scoped_lock connectionToControllerEstablishedLock(
            connectionToControllerEstablishedMutex_);
        boost::system_time const timeout = boost::get_system_time()
                                           + boost::posix_time::milliseconds(timeOutMilliSecond_);

        // there should not be any other thread waiting on connectionToControllerEvent_
        try
        {
            while (!connectionToControllerEstablished_)
            {
                asyncConnector_.connect(controllerInfo.nodeIP_,
                                        controllerInfo.nodePort_);
                if ( !connectionToControllerEvent_.timed_wait(
                            connectionToControllerEstablishedLock, timeout) )
                    throw MessageFrameworkException(SF1_MSGFRK_CONNECTION_TIMEOUT, __LINE__, __FILE__);
            }
        }
        catch (MessageFrameworkException &e)
        {
            e.output(std::cout);
            LOG(ERROR)<<"Pleas check if Controller is ready... "<<std::endl;
            exit(1);
        }

        acceptedPermissionList_.clear();
    }

    /***************************************************************************
     Description: destructor, destroy variables it if is neccessary
     ***************************************************************************/
    MessageClientFull::~MessageClientFull()
    {
        asyncConnector_.shutdown();
        ioThread_->join();
        delete ioThread_;
    }

    /****************************************************************************
     Input:
     node - remote node
     Return:
     true - connection has already been established or built successfully.
     false - if the connection to remote node cannot be established in the given time.
     Description: This function check connection to remote node, if connection
     havn't been established, this function will try to prepare the connection.
     ****************************************************************************/
    bool MessageClientFull::prepareConnection(const MessageFrameworkNode& node)
    {
        // connection to controller has not been established
        if (!connectionToControllerEstablished_)
            return false;

        if (!messageDispatcher_.isExist(node))
        {
            // if connection to sever has not been established, connect to the server;
            boost::system_time const timeout = boost::get_system_time()
                + boost::posix_time::milliseconds(timeOutMilliSecond_);
            boost::mutex::scoped_lock connectionToServerEstablishedLock(
                connectionToServerEstablishedMutex_);

            asyncConnector_.connect(node.nodeIP_, node.nodePort_);

            // maybe, the connection has been established before timed_wait
            while (!messageDispatcher_.isExist(node))
            {
                if ( !connectedToServerEvent_.timed_wait(
                            connectionToServerEstablishedLock, timeout))
                {
                    DLOG(ERROR) << getName() << " cannot connect to "
                        << node.nodeIP_ << ":" << node.nodePort_;
                    return false;
                }
            }
        }
        return true;
    }

    bool MessageClientFull::getPermissionOfService(const std::string& serviceName,
            std::map<std::string, MessageFrameworkNode>& servers)
    {
        ServicePermissionInfo servicePermissionInfo;
        std::map<std::string, ServicePermissionInfo>::const_iterator iter;

        DLOG(INFO)<< getName() << " request permission for service " << serviceName;

        // connection has not been established
        if (!connectionToControllerEstablished_) {
            DLOG(ERROR) << getName() << " acquire retrieve service permission from controller, \
                while controller hasn't been ready";
            return false;
        }

        try
        {
            sendPermissionOfServiceRequest(serviceName);

            boost::system_time const timeout = boost::get_system_time()
                + boost::posix_time::milliseconds(timeOutMilliSecond_);

            boost::mutex::scoped_lock acceptedPermissionLock(acceptedPermissionMutex_);

            while (acceptedPermissionList_.find(serviceName) == acceptedPermissionList_.end())
            {
                if (!newPermisionOfServiceEvent_.timed_wait(acceptedPermissionLock, timeout))
                {
                    DLOG(ERROR) << getName() << " timeout, cannot receive permission of Service: " << serviceName;
                    return false;
                }
            }

            // search for the permission in the list
            iter = acceptedPermissionList_.find(serviceName);
            if (iter != acceptedPermissionList_.end())
            {
                servicePermissionInfo = iter->second;
                if ( servicePermissionInfo.getServerMap().empty() )
                {
                    DLOG(ERROR) << getName() << "Service " << serviceName << " is not listed in Message Controller";
                    return false;
                }
                acceptedPermissionList_.erase(serviceName);
                servicePermissionInfo.getServerMap(servers);

                DLOG(INFO) << "getPermissionOfService returns true(newly received)";
                return true;
            }
        }
        catch (MessageFrameworkException& e)
        {
            e.output(DLOG(ERROR));
        }
        catch (boost::system::error_code& e)
        {
            DLOG(ERROR) << e;
        }
        catch (std::exception& e)
        {
            DLOG(ERROR) << e.what();
        }

        DLOG(ERROR) << "bug, fix me";
        return false;
    }

    /**
     * @brief generate unique request id
     */
    const unsigned int MessageClientFull::generateRequestId()
    {
        boost::mutex::scoped_lock lock(nxtSequentialNumberMutex_);
        return nxtSequentialNumber_++;
    }

    /******************************************************************************
     Input:
     servicePermissionInfo - it contains information of service name and the server
     serviceRequestInfo - information about request service. It contains the
     service name and its parameter values.
     Description: This function is called when we want to request result of service.
     The MessageClientFull will sends the request to either MessageController or
     MessageServer.
     ******************************************************************************/
    bool MessageClientFull::putServiceRequest(const MessageFrameworkNode& server,
            ServiceRequestInfoPtr& serviceRequestInfo, bool withReuslt)
    {
        if ( !prepareConnection(server) )
            return false;

        // Following is modified by Wei Cao, 2009-02-17
        // 1. create a semaphore for each request that need a reply.
        // 2. insert pair <requestid, semaphore into the hash map semaphoreTable_.
        // 3. then send request to MessageServer
        //
        // So when a reply arrives, it can lookup the semaphore associated with the request
        // by look up hash table semaphoreTable_, then release the semaphore to wake up any
        // thread waiting on it (the thread who calls getResultService).
        //
        // The reason I don't adopt boost::interprocess::interprocess_semaphore is the boost
        // semaphore is too costly, a test shows 5000 pairs of post() and wait() operations
        // on a boost semaphore costs nearly 10 seconds, so I write a light-weight version
        // instead, it costs only 0.3 seconds with the same test.

        std::string serviceName = serviceRequestInfo->getServiceName();
        unsigned int requestId = generateRequestId();

        DLOG(INFO) << "[Client:" << getName() << "]  generate a request id: "
        <<requestId << std::endl;

        serviceRequestInfo->setRequestId(requestId);
        // minor id ranges from 1, a minor id = 0 indicates there is only one
        // request corresponding to the request id.
        serviceRequestInfo->setMinorId(0);

        if (withReuslt)
        {
            // prepare semaphore
            {
                boost::shared_ptr<semaphore_type> semaphore = boost::shared_ptr<semaphore_type>(new semaphore_type(0));
                boost::mutex::scoped_lock semaphoreLock(semaphoreMutex_);
                semaphoreTable_[requestId] = semaphore;
            }
            // prepare result list
            {
                boost::mutex::scoped_lock resultTableLock(serviceResultMutex_);
                serviceResultTable_[requestId].resize(1);
                serviceResultCountTable_[requestId] = 1;
            }
            // insert request id to uncompleted request set
            {
                boost::mutex::scoped_lock uncompletedRequestLock(uncompletedRequestMutex_);
                uncompletedRequestSet_.insert(requestId);
            }
        }

        messageDispatcher_.sendDataToLowerLayer1(SERVICE_REQUEST_MSG, serviceRequestInfo, server);

        return true;
    }

    /******************************************************************************
     Input:
     serviceRequestInfo - it contains information of service name and the server
     Output:
     result - information about result. I contains the result of the service.
     Retur:
     true - Result is ready.
     false - result is not ready.
     Description: This function gets a result of the service that have been requested.
     The service is requested through function putServiceRequest(..)
     When the result is not ready, it returns false immediately.
     ******************************************************************************/
    bool MessageClientFull::getResultOfService(
        const ServiceRequestInfoPtr& serviceRequestInfo,
        ServiceResultPtr& serviceResult)
    {
        // connection has not been established
        if (!connectionToControllerEstablished_)
            return false;

        //the service provides no return value  @by MyungHyun - 2009-01-28

        /*if( serviceRequestInfo.getServiceResultFlag() ==
         messageframework::SERVICE_WITHOUT_RESULT)
         return false;*/

        unsigned int requestId = serviceRequestInfo->getRequestId();
        unsigned int minorId = serviceRequestInfo->getMinorId();
        std::string serviceName = serviceRequestInfo->getServiceName();

        // Modified by Wei Cao, 2009-2-19
        // 1. get the semaphore associated with the request by looking up hash table
        //    semaphoreTable_.
        // 2. timed wait on the semaphore until timeout or reply returned and semphore
        //    is released.
        // 3. delete the semaphore from semaphoreTable_.
        // 4. check wheterh request id in completedRequestSet_..
        // 5. get reply by looking up std::map serviceResultTable_.

        // semaphore may be deleted in previous invokcation
        // so, if semaphore doesn't exist, just pass it
        boost::shared_ptr<semaphore_type> semaphore;
        {
            boost::mutex::scoped_lock semaphoreLock(semaphoreMutex_);
            std::map<unsigned int, boost::shared_ptr<semaphore_type> >::const_iterator
            it = semaphoreTable_.find(requestId);
            if (it != semaphoreTable_.end() )
                semaphore = it->second;
        }
        if (semaphore != NULL)
        {
            boost::system_time const timeout = boost::get_system_time()
                                               + boost::posix_time::milliseconds(timeOutMilliSecond_);
            if ( false == semaphore->timed_wait(timeout) )
            {
                DLOG(ERROR) << "[Client:" << getName()
                << "] Reqeust id 0x" << hex << requestId << dec
                << " timeout" << std::endl;
                boost::mutex::scoped_lock acceptedPermissionLock(acceptedPermissionMutex_);
                acceptedPermissionList_.erase(serviceName);
                return false;
            }
            // if multi threads waiting on requests which owns the same request id but
            // different minor id, the thread who obtains the semaphore first has the
            // responsiblity to wake up other threads.
            // I think this operation should not have side effect, after all, semaphore
            // would never be reused.
            semaphore->post();
            boost::mutex::scoped_lock semaphoreLock(semaphoreMutex_);
            semaphoreTable_.erase(requestId);
            semaphore.reset();
        }

        // minor id equals to 0 indicats that, there is only one request corresponding to
        // the request id, so implement following steps in a optimized way.
        if (minorId == 0)
        {
            // check whether request id in completed request set
            {
                boost::mutex::scoped_lock completedRequestLock(
                    completedRequestMutex_);
                std::set<unsigned int>::iterator it =
                    completedRequestSet_.find(requestId);
                if (it == completedRequestSet_.end() )
                {
                    DLOG(ERROR)<< "[Client:" << getName() << "] invalid request "
                    << requestId << std::endl;
                    return false;
                }
                completedRequestSet_.erase(requestId);
            }
            boost::mutex::scoped_lock serviceResultLock(serviceResultMutex_);
            serviceResult = serviceResultTable_[requestId][0];
            serviceResultTable_.erase(requestId);
            serviceResultCountTable_.erase(requestId);
            return true;
        }

        // Following code is invoked only in such a situation:
        // user code sends a set of requests using batch-request interface but
        // collects the result one by one.
        // I think I should satisfy this kind of requirement, in a safe but
        // not so efficiently way, above all this kind of usage should not be
        // encouraged.

        // get a result from result list and decrease count by 1
        {
            boost::mutex::scoped_lock serviceResultLock(serviceResultMutex_);

            // check whether request id in completed request set
            // this check must be done in the same lock scope with request id deletion bellow
            {
                boost::mutex::scoped_lock completedRequestLock(
                    completedRequestMutex_);
                std::set<unsigned int>::iterator it =
                    completedRequestSet_.find(requestId);
                if (it == completedRequestSet_.end() )
                {
                    DLOG(ERROR) << "[Client:" << getName() << "] invalid request "
                    << requestId << std::endl;
                    return false;
                }
            }

            if (serviceResultTable_[requestId][minorId-1] == NULL)
                return false;
            serviceResult = serviceResultTable_[requestId][minorId -1];
            serviceResultTable_[requestId][minorId-1].reset();

            serviceResultCountTable_[requestId] --;
            if (serviceResultCountTable_[requestId] == 0)
            {
                serviceResultTable_.erase(requestId);
                serviceResultCountTable_.erase(requestId);

                // delete request id from completed request set if all results have been got away
                {
                    boost::mutex::scoped_lock completedRequestLock(
                        completedRequestMutex_);
                    completedRequestSet_.erase(requestId);
                }
            }
        }

        return true;
    }

    /**
     * @brief This function sends a request to get Permission of a service
     * @param serviceName the service name
     */
    void MessageClientFull::sendPermissionOfServiceRequest(
        const std::string& serviceName)
    {
        messageDispatcher_.sendDataToLowerLayer(PERMISSION_OF_SERVICE_REQUEST_MSG,
            serviceName, controllerNode_);
    }


    /**
     * @brief If server runs on the same machine with controller, controller may
     *        return 127.0.0.1 for that service, which is incorrect if client and
     *        controller are at different machine, so a conversion is needed here.
     */
    bool MessageClientFull::checkPermissionInfo_(
        ServicePermissionInfo& permissionInfo)
    {
        std::map<std::string, MessageFrameworkNode> agentInfoMap =
            permissionInfo.getServerMap();
        std::map<std::string, MessageFrameworkNode>::iterator it =
            agentInfoMap.begin();
        for (; it !=agentInfoMap.end(); it++ )
        {
            //change 127.0.0.1 like ip to real ip
            if ( it->second.nodeIP_ .substr(0, 3) == "127" )
            {
                MessageFrameworkNode node = it->second;
                node.nodeIP_ = controllerNode_.nodeIP_;
                it->second = node;
                permissionInfo.setServer(it->first, it->second);
            }
        }
        return true;
    }

    /******************************************************************************
     Description: This function push PermissionOfService to the internal list
     Input:
     servicePermissionInfo- the information of the PermissionOfService
     ******************************************************************************/
    void MessageClientFull::receivePermissionOfServiceResult(
        const ServicePermissionInfo& servicePermissionInfo_)
    {

        DLOG(INFO) << "[Client:" << getName()
        << "] Receive permission of " << servicePermissionInfo_.getServiceName() << std::endl;

        boost::mutex::scoped_lock acceptedPermissionLock(acceptedPermissionMutex_);
        ServicePermissionInfo  servicePermissionInfo = servicePermissionInfo_;
        checkPermissionInfo_(servicePermissionInfo);
        acceptedPermissionList_[servicePermissionInfo.getServiceName()] = servicePermissionInfo;
        acceptedPermissionLock.unlock();
        newPermisionOfServiceEvent_.notify_all();
    }

    /**
     * @brief This function sends request to the ServiceResultServer
     * @param
     * requestId - the id of service request
     * @param
     * serviceName - the service name
     * @param
     * data - the data of the service request
     * @param
     * server - the server that will receive data
     */
    void MessageClientFull::sendServiceRequest(
        const ServiceRequestInfoPtr& requestInfo,
        const MessageFrameworkNode& server)
    {
        server.display();
        // now send the message to server
        messageDispatcher_.sendDataToLowerLayer1(SERVICE_REQUEST_MSG, requestInfo,
                server);
    }

    /*********************************************************************************
     Description: This function updates result of a requested service.
     It put the result in the result table. Later, the result is retrieved through
     getResultOfService(function)
     Input:
     requestId - the request Id
     serviceResult - the result of the requested service
     *********************************************************************************/
    void MessageClientFull::receiveResultOfService(const ServiceResultPtr& result)
    {

        DLOG(INFO) << "[Client:" << getName() << "] Receive result of request " << result->getRequestId() << std::endl;
        //result->display( LOG(INFO) );

        //cout<<"!!!!! wps"<<endl;
        //check whether request id in uncompleted request set
        //then remove request id from uncompleted request set
        unsigned int requestId = result->getRequestId();
        {
            boost::mutex::scoped_lock uncompletedRequestLock(
                uncompletedRequestMutex_);
            std::set<unsigned int>::iterator it =
                uncompletedRequestSet_.find(requestId);
            if (it == uncompletedRequestSet_.end() )
            {
                DLOG(ERROR) << "[Client:" << getName() << "] invalid request "
                << requestId << std::endl;
                throw MessageFrameworkException(SF1_MSGFRK_LOGIC_ERROR, __LINE__, __FILE__);
            }
            uncompletedRequestSet_.erase(requestId);
        }
        {
            boost::mutex::scoped_lock serviceResultLock(serviceResultMutex_);
            serviceResultTable_[requestId][0] = result;
        }

        // insert results into result table
        {
            boost::mutex::scoped_lock serviceResultLock(serviceResultMutex_);
            int expectedResultCount = serviceResultCountTable_[requestId];

            int resultCount = 1;
            if (result->getMinorId() != 0)
                resultCount= result->getBufferNum();
            if (expectedResultCount != resultCount)
            {
                DLOG(ERROR) << "[Client:" << getName() << "] processing request "
                << requestId << " expect " << expectedResultCount
                << " results " << " but receive " << resultCount
                << " results " << std::endl;
                throw MessageFrameworkException(SF1_MSGFRK_LOGIC_ERROR, __LINE__, __FILE__);
            }
            unsigned int minorId = result->getMinorId();

            if (minorId == 0)
            {
                serviceResultTable_[requestId][0] = result;
            }
            else
            {

                for (int i = 0; i < resultCount; i++)
                {
                    ServiceResultPtr serviceResult(new ServiceResult);
                    minorId = i+1;
                    serviceResult->setServiceName(result->getServiceName());
                    serviceResult->setRequestId(result->getRequestId());
                    serviceResult->setMinorId(i+1);
                    serviceResult->pushBuffer(result->getBuffer(i));
                    serviceResultTable_[requestId][minorId-1] = serviceResult;

                }
            }
        }

        //result is ready, insert request id into completed request table
        {
            boost::mutex::scoped_lock completedRequestLock(completedRequestMutex_);
            completedRequestSet_.insert(requestId);
        }

        // When a reply arrives, find the semaphore associated with the request, then
        // release the semaphore to wake up any threads waiting on it ( most probably
        // the thread who calls getServiceResult )
        boost::shared_ptr<semaphore_type> semaphore;
        {
            boost::mutex::scoped_lock semaphoreLock(semaphoreMutex_);
            std::map<unsigned int, boost::shared_ptr<semaphore_type> >::const_iterator
            it = semaphoreTable_.find(requestId);
            if (it != semaphoreTable_.end() )
                semaphore = it->second;
        }
        if (semaphore != NULL)
            semaphore->post();
    }

    /**
     * @brief This function create a new AsyncStream that is based on tcp::socket
     */
    AsyncStream* MessageClientFull::createAsyncStream(
        boost::shared_ptr<tcp::socket> sock)
    {
        tcp::endpoint endpoint = sock->remote_endpoint();

        if (!connectionToControllerEstablished_)
        {
            DLOG(INFO) << "Accept new connection from controller, Remote IP = "
                << endpoint.address().to_string() << endpoint.port();

            // this must be a connection to controller
            AsyncStream* pNewSocket;
            {
                boost::mutex::scoped_lock connectionToControllerEstablishedLock(
                    connectionToControllerEstablishedMutex_);
                pNewSocket = new AsyncStream(&messageDispatcher_, sock);
                connectionToControllerEstablished_ = true;
            }
            connectionToControllerEvent_.notify_all();
            return pNewSocket;

        }
        else
        {

            DLOG(INFO) << "Accept new connection from server, Remote IP = "
                << endpoint.address().to_string() << endpoint.port();

            AsyncStream* pNewSocket;
            {
                boost::mutex::scoped_lock connectionToServerEstablishedLock(
                    connectionToServerEstablishedMutex_);
                pNewSocket = new AsyncStream(&messageDispatcher_, sock);
            }
            // the connection is made with server
            connectedToServerEvent_.notify_all();
            return pNewSocket;
        }
    }
}// end of messageframework

