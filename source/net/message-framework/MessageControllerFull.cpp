///
/// @file MessageControllerFull.cpp
/// @date 8/5/2008
/// @author TuanQuang Nguyen
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

			
				//ServicePermissionInfo permission;
				//permission.setServiceName(serviceInfo.getServiceName());
				//permission.setPermissionFlag(serviceInfo.getPermissionFlag());
				//permission.setServer(serviceInfo.getServer());
				//permission.setServiceResultFlag( serviceInfo.getServiceResultFlag() ); // @by MyungHyun - 2009-01-28
				// send permission message to client
				
				
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
 Description: This function retrieves information of a service. It returns false
 if the service is not found.
 Input:
 serviceName - the service name
 serviceInfo - the information of service
 ******************************************************************************/
/*bool MessageControllerFull::getServiceInfo(const std::string& serviceName,
		ServiceInfo& serviceInfo) {
	boost::unordered_map<std::string, ServiceInfo>::iterator it;
	it = availableServiceList_.find(serviceName);
	if (it!=availableServiceList_.end()) {
		serviceInfo = (it->second);
		return true;
	}

	return false;
}*/

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
 Description: This function requests a result of service. It sends a request
 message to the server.
 ******************************************************************************/
/*void MessageControllerFull::forwardServiceRequest(const ServiceRequestInfo& requestInfo)
 {
 try
 {
 unsigned int requestId = requestInfo.getRequestId();
 std::string serviceName = requestInfo.getServiceName();

 ServiceInfo serviceInfo;
 if(!getServiceInfo(serviceName, serviceInfo))
 throw MessageFrameworkException(SF1_MSGFRK_UNKNOWN_ERROR, __LINE__, __FILE__);

 // forward request to server
 sendServiceRequest(requestId, serviceName,
 requestInfo.getParameterList(),
 serviceInfo.getServer());

 #ifdef SF1_DEBUG
 std::cout << "[Controller:" << getName();
 std::cout << "] Successfully send service request [Id = " << requestId;
 std::cout << "]  " << serviceName << " to " << serviceInfo.getServer().nodeName_ << std::endl;
 #endif
 }
 catch(MessageFrameworkException& e)
 {
 e.output(std::cerr);
 }
 catch (boost::system::error_code& e)
 {
 std::cerr << "Exception: " << e << std::endl;
 }
 catch (std::exception& e)
 {
 std::cerr << "Exception: "<< e.what() << std::endl;
 }

 }*/

/******************************************************************************
 Description: This function processes all waiting service requests from MessageClient.
 It will deliver these messages MessageServers.
 ******************************************************************************/
/*	void MessageControllerFull::processServiceRequestFromClient(void)
 {
 try
 {
 std::pair<MessageFrameworkNode, ServiceRequestInfo> requestItem;

 boost::mutex::scoped_lock serviceRequestQueueLock(serviceRequestQueueMutex_);
 while(true)
 {
 // Waiting for newServiceRequestEvent_
 // It is waken up if controller receives a service request
 // from client
 newServiceRequestEvent_.wait(serviceRequestQueueLock);
 #ifdef _LOGGING_
 WriteToLog("log.log", "============= processServiceRequestFromClient ===========");
 #endif
 while(serviceRequestQueue_.size() > 0)
 {
 requestItem = serviceRequestQueue_.front();
 serviceRequestQueue_.pop();
 serviceRequestQueueLock.unlock();

 {
 // Put the request into  waitingForResultMessageQueue_.
 // The controller waits for results from MessageServer
 boost::mutex::scoped_lock waitingForResultMessageQueueLock(
 waitingForResultMessageQueueMutex_);
 waitingForResultMessageQueue_.insert(
 std::pair<unsigned int, MessageFrameworkNode>(
 requestItem.second.getRequestId(),
 requestItem.first));
 }
 // send the request to server
 forwardServiceRequest(requestItem.second);

 serviceRequestQueueLock.lock();
 }// end of while(totalRequest > 0 || !serviceRequestQueue_.empty())
 }// end of while(true)
 }
 catch(MessageFrameworkException& e)
 {
 e.output(std::cerr);
 }
 catch (boost::system::error_code& e)
 {
 std::cerr << "Exception: " << e << std::endl;
 }
 catch (std::exception& e)
 {
 std::cerr << "Exception: "<< e.what() << std::endl;
 }
 }*/

/******************************************************************************
 Description: This function processes all the coming results from MessageServer.
 It will deliver the results to the MessageClient.
 ******************************************************************************/
/*void MessageControllerFull::processServiceResultFromServer(void)
 {
 try
 {
 // now , add the result to the result queue
 std::pair<MessageFrameworkNode, ServiceResult> resultItem;
 ServiceRequestInfo request;

 boost::mutex::scoped_lock resultQueueLock(resultQueueMutex_);
 while(true)
 {
 // waiting for newResultEvent_
 // It is waken up if controller receives a service result
 // from server
 newServiceResultEvent_.wait(resultQueueLock);
 #ifdef _LOGGING_
 WriteToLog("log.log", "============= processServiceResultFromServer ===========");
 #endif
 while(resultQueue_.size() > 0)
 {
 resultItem = resultQueue_.front();
 resultQueue_.pop();
 resultQueueLock.unlock();

 // send result to the client
 request.setRequestId(resultItem.second.getRequestId());
 sendResultOfService(resultItem.first, request, resultItem.second);
 #ifdef SF1_DEBUG
 std::cout << "[Controller:" << getName();
 std::cout << "] Successfully forward result of request " << resultItem.second.getRequestId() << std::endl;
 #endif

 resultQueueLock.lock();
 }
 }
 }
 catch(MessageFrameworkException& e)
 {
 e.output(std::cerr);
 }
 catch (boost::system::error_code& e)
 {
 std::cerr << "Exception: " << e << std::endl;
 }
 catch (std::exception& e)
 {
 std::cerr << "Exception: "<< e.what() << std::endl;
 }


 }*/

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
/*	void MessageControllerFull::sendServiceRequest(unsigned requestId,
 const std::string& serviceName,
 const std::vector<boost::shared_ptr<VariantType> >& data,
 const MessageFrameworkNode& server)
 {
 ServiceInfo serviceInfo;
 if(!getServiceInfo(serviceName, serviceInfo))
 throw MessageFrameworkException(SF1_MSGFRK_DATA_NOT_FOUND, __LINE__, __FILE__);

 // construct the request message
 ServiceMessage message;
 message.setRequestId(requestId);
 message.setServiceName(serviceName);
 for(size_t i = 0; i < data.size(); i++)
 {
 message.appendData(data[i]);
 }

 // send to network layer, the network layer then sends data to ServiceResultServer
 messageDispatcher_.sendDataToLowerLayer(SERVICE_REQUEST_MSG, message, server);
 }*/

/******************************************************************************
 Description: This function updates result of a requested service.
 It puts the result in the result queue.
 requestId - id of the request
 serviceName - the name of service
 data - result of the service
 ******************************************************************************/
/*	void MessageControllerFull::receiveResultOfService(unsigned int requestId,
 const std::string& serviceName,
 const std::vector<boost::shared_ptr<VariantType> >& data)
 {
 try
 {
 boost::mutex::scoped_lock waitingForResultMessageQueueLock(waitingForResultMessageQueueMutex_);
 std::map<unsigned int, MessageFrameworkNode >::iterator iter;
 iter = waitingForResultMessageQueue_.find(requestId);
 if(iter == waitingForResultMessageQueue_.end())
 throw MessageFrameworkException(SF1_MSGFRK_DATA_NOT_FOUND, __LINE__, __FILE__);

 ServiceResult serviceResult;
 serviceResult.setRequestId(requestId);
 serviceResult.setServiceName(serviceName);
 for(size_t i = 0; i < data.size(); i++)
 serviceResult.appendServiceData(data[i]);

 // put the result in the result queue. The result queue will be processed in
 // processResultFromServer(...)
 boost::mutex::scoped_lock resultQueueLock(resultQueueMutex_);
 resultQueue_.push(std::pair<MessageFrameworkNode, ServiceResult>(iter->second, serviceResult));
 resultQueueLock.unlock();

 // This request does not needs to wait for result
 waitingForResultMessageQueue_.erase(requestId);
 waitingForResultMessageQueueLock.unlock();

 // notify a new result event
 newServiceResultEvent_.notify_all();
 }
 catch(MessageFrameworkException& e)
 {
 e.output(std::cerr);
 }
 catch (boost::system::error_code& e)
 {
 std::cerr << e << std::endl;
 }
 catch (std::exception& e)
 {
 std::cerr << "Exception: "<< e.what() << std::endl;
 }
 }*/

/**
 * @brief This function put the request for a service result in the waiting list.
 */
/*void MessageControllerFull::receiveServiceRequest(const MessageFrameworkNode& requester,
 unsigned int requestId,
 const std::string& serviceName,
 const std::vector<boost::shared_ptr<VariantType> >& data)
 {

 // get information of server
 if(availableServiceList_.find(serviceName) == availableServiceList_.end())
 throw MessageFrameworkException(SF1_MSGFRK_UNKNOWN_ERROR, __LINE__, __FILE__);
 #ifdef SF1_DEBUG
 std::cout << "[Controller:" << getName();
 std::cout << "] Receive service request of service " << serviceName;
 std::cout << " from " << requester.nodeName_ << std::endl;
 #endif

 // construct the request using input parameter
 ServiceRequestInfo requestInfo;
 requestInfo.setRequestId(requestId);
 requestInfo.setServiceName(serviceName);
 for(size_t i = 0; i < data.size(); i++)
 requestInfo.appendParameter(data[i]);

 // add the the serviceRequestQueue_
 boost::mutex::scoped_lock serviceRequestQueueLock(serviceRequestQueueMutex_);
 serviceRequestQueue_.push(std::pair<MessageFrameworkNode, ServiceRequestInfo>(requester, requestInfo));
 serviceRequestQueueLock.unlock();

 // Notify that new request is coming, newServiceRequestEvent_ makes the
 // MessageControllerFull sends the request to the server
 newServiceRequestEvent_.notify_all();

 }
 */
/**
 * This function replies to a request from ServiceResultRequester. I will sends
 * ServiceResult to the ServiceResultRequester.
 */
/*	void MessageControllerFull::sendResultOfService(const MessageFrameworkNode& requester,
 const ServiceRequestInfo& requestInfo,
 const ServiceResult& result)
 {
 ServiceMessage message;

 message.setRequestId(requestInfo.getRequestId());
 message.setServiceName(result.getServiceName());
 for(size_t i = 0; i < result.getServiceResult().size(); i++)
 message.appendData(result.getServiceResult()[i]);

 messageDispatcher_.sendDataToLowerLayer(SERVICE_RESULT_MSG, message, requester);
 }*/

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

