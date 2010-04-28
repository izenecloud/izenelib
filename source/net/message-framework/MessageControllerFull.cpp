///
/// @file MessageControllerFull.cpp
/// @date 8/5/2008
/// @author TuanQuang Nguyen
///
///

/**************** Include module header files *******************/
// #include <BasicMessage.h>
#include <net/message-framework/MessageControllerFull.h>
#include <net/message-framework/MessageFrameworkConfiguration.h>
#include <net/message-framework/ServiceMessage.h>
#include <net/message-framework/MessageType.h>
#include <net/message-framework/ServiceRegistrationMessage.h>
#include <net/message-framework/ServiceRegistrationReplyMessage.h>
#include <net/message-framework/ClientIdReplyMessage.h>
#include <net/message-framework/ServicePermissionInfo.h>

/**************** Include boost header files *******************/
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

/**************** Include std header files *******************/
#include <iostream>

/**************** Include sf1lib header files *******************/

const static string AvailServiceFileNamePrefix = "_ctr_services.dat";

const static string getAvailServiceSdbName(unsigned int port) {
    return "_ctr_services." + boost::lexical_cast<string>(port) + ".dat";
}

namespace messageframework {

/**
 * static member variables
 */
std::string MessageControllerFullSingleton::controllerName_("");

unsigned int MessageControllerFullSingleton::controllerPort_ = 0;

MessageControllerFull* MessageControllerFullSingleton::messageController_(NULL);

MessageControllerFull::MessageControllerFull(
        const std::string& controllerName,
		unsigned int servicePort) :
    ownerManagerName_(controllerName),
    availableServiceList_(getAvailServiceSdbName(servicePort)),
	messageDispatcher_(this, this, this, this, this),
	asyncStreamManager_(messageDispatcher_),
	asyncAcceptor_(io_service_, asyncStreamManager_)
{
	servicePort_ = servicePort;
	ipAddress_ = getLocalHostIp(io_service_);

	asyncAcceptor_.listen(servicePort_);

	timeOutMilliSecond_ = 1000;

	// start from 1, so 0 is a invalid client id
	nxtClientId_ = 1;

	availableServiceList_.open();
	if(availableServiceList_.numItems() > 0) {
        DLOG(WARNING) << availableServiceList_.numItems()
         << " pieces of service registration information found in "
         << getAvailServiceSdbName(servicePort);

         izenelib::sdb::ordered_sdb<std::string, ServicePermissionInfo>::SDBCursor
         cursor = availableServiceList_.get_first_locn();
         std::string serviceName;
         ServicePermissionInfo permissionInfo;
         while(availableServiceList_.get(cursor, serviceName, permissionInfo)) {
#ifndef NDEBUG
             permissionInfo.display(DLOG(WARNING));
#endif
             availableServiceList_.seq(cursor);
         }
	}

    stop_ = false;
	workThread1_ = new boost::thread(boost::bind(&MessageControllerFull::processServiceRegistrationRequest, this));
	workThread2_ = new boost::thread(boost::bind(&MessageControllerFull::processServicePermissionRequest, this));
}

MessageControllerFull::~MessageControllerFull() {
    if(!stop_) shutdown();
}

void MessageControllerFull::run() {
	boost::thread* ioThread = new boost::thread(boost::bind(&boost::asio::io_service::run, &io_service_));
	std::cout << "Controller run" << std::endl;
	ioThread->join();
	delete ioThread;
}

void MessageControllerFull::shutdown() {
	asyncAcceptor_.shutdown();
	asyncStreamManager_.shutdown();

    stop_ = true;

    newRegistrationEvent_.notify_all();
    workThread1_->join();
    delete workThread1_;

    newPermissionRequestEvent_.notify_all();
    workThread2_->join();
    delete workThread2_;

    availableServiceList_.flush();
    availableServiceList_.close();

    std::cout << "Controller shutdown" << std::endl;
}

/******************************************************************************
 Description: This function processes all the waiting service registration requests.
 It registers all the services from MessageServer (IP address, port number,
 a list of service which can be provided by the MessageServer).
 *******************************************************************************/
void MessageControllerFull::processServiceRegistrationRequest(void) {
	try
	{
		boost::mutex::scoped_lock serviceRegistrationRequestQueueLock(
				serviceRegistrationRequestQueueMutex_);

		while(true)
		{
			newRegistrationEvent_.wait(serviceRegistrationRequestQueueLock);
			if(stop_) {
			    DLOG(INFO) << "stop service registration request process";
			    return;
			}

			DLOG(INFO) << "=============processServiceRegistrationRequest===========";

			while(!serviceRegistrationRequestQueue_.empty())
			{
				std::pair<ServiceRegistrationMessage, MessageFrameworkNode> request =
				serviceRegistrationRequestQueue_.front();
				serviceRegistrationRequestQueue_.pop();
				serviceRegistrationRequestQueueLock.unlock();

				string serviceName = request.first.getServiceInfo().getServiceName();
				string agentInfo = request.first.getAgentInfo();
				MessageFrameworkNode server = request.first.getServiceInfo().getServer();
				if( server.nodeIP_.substr(0, 3) == "127" )
					server.nodeIP_ = request.second.nodeIP_;
				{
					// check if the sevice has been registered
					boost::mutex::scoped_lock availableServiceListLock(availableServiceListMutex_);

                    std::cout <<"ServiceRegistrationRequest: " << serviceName << ","
                        << agentInfo << std::endl;

					ServicePermissionInfo permissionInfo;
					if( availableServiceList_.get(serviceName, permissionInfo) )
					{
						DLOG(WARNING) << "[Controller:" << getName() << "] ServiceInfo "
                            << serviceName << " already exists."
                            << "New data overwrites old data.";

						permissionInfo.setServer(request.first.getAgentInfo(), server);

					} else
					{
						permissionInfo.setServiceName(serviceName);
						permissionInfo.setServer(request.first.getAgentInfo(), server);
					}

                    availableServiceList_.update(serviceName, permissionInfo);
                    availableServiceList_.flush();

					sendServiceRegistrationResult(request.second, request.first.getServiceInfo(), true);
				}

				DLOG(INFO)  << "[Controller:" << getName() << "]"
                            << " ServiceInfo [" << request.first.getServiceName()
                            << ", " << request.first.getServer().nodeIP_ << ":"
                            << request.first.getServer().nodePort_
                            << "] is successfully registerd";

				serviceRegistrationRequestQueueLock.lock();
			}
		}// end of while true
	}
	catch(MessageFrameworkException& e)
	{
		e.output(std::cout);
	}
	catch (boost::system::error_code& e)
	{
		std::cerr << "Exception: " << e << std::endl;
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: "<< e.what() << std::endl;
	}
}

void MessageControllerFull::processServicePermissionRequest(void) {
	try
	{
		std::pair<std::string, MessageFrameworkNode> requestItem;
		ServiceInfo serviceInfo;

		boost::mutex::scoped_lock permissionRequestQueueLock(servicePermissionRequestQueueMutex_);
		while(true)
		{
			newPermissionRequestEvent_.wait(permissionRequestQueueLock);
			if(stop_) {
			    DLOG(INFO) << "stop service permission request process";
			    return;
			}
			DLOG(INFO) << "============= processServicePermissionRequest ===========\n";

			// process all the waiting requests
			while(!servicePermissionRequestQueue_.empty())
			{
				requestItem = servicePermissionRequestQueue_.front();
				servicePermissionRequestQueue_.pop();

				permissionRequestQueueLock.unlock();

                // check if the sevice has been registered
                boost::mutex::scoped_lock availableServiceListLock(availableServiceListMutex_);

                ServicePermissionInfo permissionInfo;
                if( availableServiceList_.get(requestItem.first, permissionInfo) )
				{
					sendPermissionOfServiceResult(requestItem.second, permissionInfo );

				} else
				{
					permissionInfo.setServiceName(requestItem.first);
					//serviceInfo.setPermissionFlag(UNKNOWN_PERMISSION_FLAG);

					DLOG(ERROR) << " [Controller:" << getName()
                                << "] Service " << requestItem.first
                                << " is not listed.";

					sendPermissionOfServiceResult(requestItem.second, permissionInfo );
					//cout<<"!!! no listed"<<endl;
					//availableServiceList_[requestItem.first].display();
				}

				DLOG(INFO) << "[Controller:" << getName() << "] Permission of " << requestItem.first
                        << " is successfully sent to " << requestItem.second.nodeIP_ << ":"
                        << requestItem.second.nodePort_;

				permissionRequestQueueLock.lock();
			}// end of while(!servicePermissionRequestQueue_.empty())
		}// end of while(true)
	}
	catch(MessageFrameworkException& e)
	{
        DLOG(ERROR) << e.getString();
	}
	catch (boost::system::error_code& e)
	{
		DLOG(ERROR) << e;
	}
	catch (std::exception& e)
	{
		DLOG(ERROR) << e.what();
	}

}

/******************************************************************************
 Description: When a new request of get client id comes, this function is called
 in order to notify the server
 ******************************************************************************/
void MessageControllerFull::receiveClientIdRequest(
		const MessageFrameworkNode& requester) {
	{
		boost::mutex::scoped_lock lock(nxtClientIdMutex_);
		if (nxtClientId_ == MF_FULL_MAX_CLIENT_ID) {
			DLOG(ERROR) << "Max Client ID reached" << std::endl;
			throw MessageFrameworkException(SF1_MSGFRK_LOGIC_ERROR, __LINE__, __FILE__);
		}
		DLOG(INFO) << "nxtClientId_="
				<<nxtClientId_ << std::endl;
		sendClientIdResult(requester, nxtClientId_);
		nxtClientId_ ++;
	}
}

/******************************************************************************
 Description: This function replies to a request from ClientIdRequester
 ******************************************************************************/
void MessageControllerFull::sendClientIdResult(
		const MessageFrameworkNode& requester, const int& clientId) {
	ClientIdReplyMessage message(clientId);
	messageDispatcher_.sendDataToLowerLayer(CLIENT_ID_REPLY_MSG, message,
			requester);
}

/******************************************************************************
 Description: This function put a request to get  permission of service into the
 waiting queue. The requests in the waitiing queue will be processed in function
 processServicePermissionRequest().
 ******************************************************************************/
void MessageControllerFull::receivePermissionOfServiceRequest(
		const MessageFrameworkNode& requester, const std::string& serviceName) {
	DLOG(INFO) << "[Controller:" << getName() << "] Receive permission request of "
            << serviceName << " from " << requester.nodeName_;

	{
		boost::mutex::scoped_lock lock(servicePermissionRequestQueueMutex_);
		servicePermissionRequestQueue_.push(std::pair<std::string,
            MessageFrameworkNode>(serviceName, requester));
	}
	newPermissionRequestEvent_.notify_all();
}

/******************************************************************************
 Description: This function inserts a ServiceInfo to the serviceRegistrationRequestQueue_.
 It returns false if the data alread exists in the queue.
 Input:
 serviceInfo - data to add
 ******************************************************************************/
void MessageControllerFull::receiveServiceRegistrationRequest(
        const MessageFrameworkNode& localEndPoint,
        const ServiceRegistrationMessage& registMessage)
{
	DLOG(INFO) << "[Controller:" << getName() << "] Receive registration request";
    {
        boost::mutex::scoped_lock lock(serviceRegistrationRequestQueueMutex_);
        serviceRegistrationRequestQueue_.push( std::pair<ServiceRegistrationMessage,
            MessageFrameworkNode>(registMessage, localEndPoint));
    }
    newRegistrationEvent_.notify_all();
}

/**
 * @brief This function replies to a registration request from ServiceRegistrationRequester. It
 * will sends registration result to the ServiceRegistrationRequester.
 * @param localEndPoint the end point that connects to the ServiceRegistrationClient.
 * @param serviceInfo the service information to register
 * @param success the registration result
 */
void MessageControllerFull::sendServiceRegistrationResult(
        const MessageFrameworkNode& localEndPoint,
        const ServiceInfo& serviceInfo, bool success)
{
	ServiceRegistrationReplyMessage message(serviceInfo.getServiceName(), success);
	messageDispatcher_.sendDataToLowerLayer(SERVICE_REGISTRATION_REPLY_MSG, message, localEndPoint);
}

/**
 * @brief This function replies to a request from PermissionRequester
 */
void MessageControllerFull::sendPermissionOfServiceResult(
        const MessageFrameworkNode& requester,
        const ServicePermissionInfo & permission)
{
	messageDispatcher_.sendDataToLowerLayer(PERMISSION_OF_SERVICE_REPLY_MSG, permission, requester);
}

}// end of namespace messageframework
