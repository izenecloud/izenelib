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

/**************** Include std header files *******************/
#include <iostream>

/**************** Include sf1lib header files *******************/

namespace messageframework {
//static member variables
std::string MessageControllerFullSingleton::controllerName_("");
unsigned int MessageControllerFullSingleton::controllerPort_ = 0;
boost::scoped_ptr<MessageControllerFull>
		MessageControllerFullSingleton::messageController_(0);
boost::once_flag MessageControllerFullSingleton::flag= BOOST_ONCE_INIT;

MessageControllerFull::MessageControllerFull(const std::string& controllerName,
		unsigned int servicePort) :
	messageDispatcher_(this, this, this, this, this), asyncConnector_(this,
			io_service_) {
	ownerManager_ = controllerName;

	servicePort_ = servicePort;
	asyncConnector_.listen(servicePort_);
	ipAddress_ = getLocalHostIp(io_service_);

	// timeout is 1 second
	timeOutMilliSecond_ = 1000;

	//serviceResultReply_ = false;

	// start from 1, so 0 is a invalid client id
	nxtClientId_ = 1;

	// create thread for I/O operations
	ioThread_ = new boost::thread(boost::bind(&boost::asio::io_service::run, &io_service_));
}

MessageControllerFull::~MessageControllerFull() {
	asyncConnector_.shutdown();
	ioThread_->join();
	delete ioThread_;
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
#ifdef _LOGGING_
			WriteToLog("log.log", "=============processServiceRegistrationRequest===========");
#endif
			while(!serviceRegistrationRequestQueue_.empty())
			{
				std::pair<ServiceRegistrationMessage, MessageFrameworkNode> request =
				serviceRegistrationRequestQueue_.front();
				serviceRegistrationRequestQueue_.pop();
				serviceRegistrationRequestQueueLock.unlock();

				{
					// check if the sevice has been registered
					string serviceName = request.first.getServiceInfo().getServiceName();
					MessageFrameworkNode server = request.first.getServiceInfo().getServer();
					boost::mutex::scoped_lock availableServiceListLock(availableServiceListMutex_);
					
					std::cout<<"ServiceRegistrationRequest: "<<serviceName<<std::endl;
					if( availableServiceList_.find(serviceName) != availableServiceList_.end() )
					{
#ifdef SF1_DEBUG
						std::cout << "[Controller:" << getName();
						std::cout << "] ServiceInfo " << request.first.getServiceName() << " already exists.";
						std::cout << "New data overwrites old data." << std::endl;
	#endif
						
						availableServiceList_[serviceName].setServer(request.first.getAgentInfo(), server);
					} else
					{
						ServicePermissionInfo permissionInfo;
						permissionInfo.setServiceName(serviceName);						
						permissionInfo.setServer(request.first.getAgentInfo(), server);
						availableServiceList_[serviceName] = permissionInfo;
					}
					
					//availableServiceList_[serviceName].display();
					sendServiceRegistrationResult(request.second, request.first.getServiceInfo(), true);
				}

#ifdef SF1_DEBUG
				std::cout << "[Controller:" << getName() << "] ServiceInfo [" << request.first.getServiceName();
				std::cout << ", " << request.first.getServer().nodeIP_ << ":";
				std::cout << request.first.getServer().nodePort_ << "] is successfully registerd." << std::endl;
#endif
				// connection to server
				if(!messageDispatcher_.isExist(request.first.getServer()))
				{
					asyncConnector_.connect(request.first.getServer().nodeIP_,
							request.first.getServer().nodePort_);
				}
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

/******************************************************************************
 Description: This function processes all the waiting service permission requests.
 It determines whether it is available to process the service and issues
 permission to the MessageClient. If the service needs to use the direct
 connection, MessageControllerFull sends the permission to the MessageServer.
 ******************************************************************************/

bool MessageControllerFull::checkAgentInfo_(ServicePermissionInfo& permissionInfo)
{	
	const std::map<std::string, MessageFrameworkNode>& agentInfoMap =
			permissionInfo.getServerMap();
	std::map<std::string, MessageFrameworkNode>::const_iterator it =
			agentInfoMap.begin();

	for (; it != agentInfoMap.end(); it++) {
		if ( !messageDispatcher_.isExist(it->second) ) {
			DLOG(ERROR)<<"ServicePermissionInfo:"<<it->first<<" -> server:"
					<<it->second<<"not exists";
			permissionInfo.removeServer(it->first);
		}
	}
	if (it != agentInfoMap.end() )
		return false;
	return true;

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
#ifdef _LOGGING_
			WriteToLog("log.log", "============= processServicePermissionRequest ===========");
#endif
			// process all the waiting requests
			while(!servicePermissionRequestQueue_.empty())
			{
				requestItem = servicePermissionRequestQueue_.front();
				servicePermissionRequestQueue_.pop();

				permissionRequestQueueLock.unlock();
				
				if( availableServiceList_.find(requestItem.first) != availableServiceList_.end() )
				{	
				   checkAgentInfo_( availableServiceList_[requestItem.first] );
					sendPermissionOfServiceResult(requestItem.second, availableServiceList_[requestItem.first] );
					// to improve performance, direct connection to
					// server is always made
					//serviceInfo.setPermissionFlag(SERVE_AT_SERVER);
					// serviceInfo.setServer(ipAddress_, servicePort_);

				} else
				{		
					ServicePermissionInfo permission;
					permission.setServiceName(requestItem.first);
					//serviceInfo.setPermissionFlag(UNKNOWN_PERMISSION_FLAG);
#ifdef SF1_DEBUG
					std::cout << " [Controller:" << getName();
					std::cout << "] Service " << requestItem.first;
					std::cout << " is not listed." << std::endl;
#endif
					sendPermissionOfServiceResult(requestItem.second, permission );
				}
				
#ifdef SF1_DEBUG
				std::cout << "[Controller:" << getName();
				std::cout << "] Permission of " << requestItem.first;
				std::cout << " is successfully sent to ";
				std::cout << requestItem.second.nodeIP_ << ":" << requestItem.second.nodePort_ << std::endl;
#endif
				permissionRequestQueueLock.lock();
			}// end of while(!servicePermissionRequestQueue_.empty())
		}// end of while(true)
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

/******************************************************************************
 Description: When a new request of get client id comes, this function is called
 in order to notify the server
 ******************************************************************************/
void MessageControllerFull::receiveClientIdRequest(
		const MessageFrameworkNode& requester) {
	{
		boost::mutex::scoped_lock lock(nxtClientIdMutex_);
		if (nxtClientId_ == MF_FULL_MAX_CLIENT_ID) {
			std::cout << "Max Client ID reached" << std::endl;
			throw MessageFrameworkException(SF1_MSGFRK_LOGIC_ERROR, __LINE__, __FILE__);
		}
		std::cout << "DEBUG: MessageControllerFull:: dispatch nxtClientId_="
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
#ifdef SF1_DEBUG
	std::cout << "[Controller:" << getName() << "] Receive permission request of ";
	std::cout << serviceName << " from " << requester.nodeName_ << std::endl;
#endif
	{
		boost::mutex::scoped_lock lock(servicePermissionRequestQueueMutex_);
		servicePermissionRequestQueue_.push(std::pair<std::string, MessageFrameworkNode>(serviceName, requester));
	}
	newPermissionRequestEvent_.notify_all();
}


/******************************************************************************
 Description: This function inserts a ServiceInfo to the serviceRegistrationRequestQueue_.
 It returns false if the data alread exists in the queue.
 Input:
 serviceInfo - data to add
 ******************************************************************************/
void MessageControllerFull::receiveServiceRegistrationRequest(const MessageFrameworkNode& localEndPoint,
const ServiceRegistrationMessage& registMessage)
{
	try
	{
		{
			boost::mutex::scoped_lock lock(serviceRegistrationRequestQueueMutex_);
			serviceRegistrationRequestQueue_.push(
			std::pair<ServiceRegistrationMessage, MessageFrameworkNode>(registMessage, localEndPoint));
		}
		newRegistrationEvent_.notify_all();
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

/**
 * @brief This function replies to a registration request from ServiceRegistrationRequester. It
 * will sends registration result to the ServiceRegistrationRequester.
 * @param localEndPoint the end point that connects to the ServiceRegistrationClient.
 * @param serviceInfo the service information to register
 * @param success the registration result
 */
void MessageControllerFull::sendServiceRegistrationResult(const MessageFrameworkNode& localEndPoint, const ServiceInfo& serviceInfo,
bool success)
{
	ServiceRegistrationReplyMessage message(serviceInfo.getServiceName(), success);

	messageDispatcher_.sendDataToLowerLayer(SERVICE_REGISTRATION_REPLY_MSG, message, localEndPoint);
}

/**
 * @brief This function replies to a request from PermissionRequester
 */
void MessageControllerFull::sendPermissionOfServiceResult(const MessageFrameworkNode& requester,
const ServicePermissionInfo & permission)
{		
	messageDispatcher_.sendDataToLowerLayer(PERMISSION_OF_SERVICE_REPLY_MSG,
	permission, requester);
}

/**
 * @brief This function create a new AsyncStream that is based on tcp::socket
 */
AsyncStream* MessageControllerFull::createAsyncStream(boost::shared_ptr<tcp::socket> sock)
{
	// tcp::endpoint endpoint = sock->local_endpoint();
	tcp::endpoint endpoint = sock->remote_endpoint();
	std::cout << "Remote IP = " << endpoint.address().to_string();
	std::cout << ", port = " << endpoint.port() << std::endl;

	return new AsyncStream(&messageDispatcher_, sock);
}

}// end of namespace messageframework

