///
///  @file : MessageServerFull.cpp
///  @date : 8/5/2008
///  @author : TuanQuang Nguyen
///


/**************** Include module header files *******************/
#include <net/message-framework/MessageServerFull.h>
#include <net/message-framework/MessageFrameworkConfiguration.h>
#include <net/message-framework/ServiceRegistrationMessage.h>
#include <net/message-framework/MessageType.h>
#include <net/message-framework/ServiceMessage.h>

/**************** Include std header files *******************/
#include <iostream>

/**************** Include boost header files *******************/

namespace messageframework {

/***************************************************************************
 Description: Initialize variables with default values. It initialize resources
 for communication.
 Input:
 serverName - name of server
 servicePort - port  of service that serves results of services
 controllerInfo - information of controller
 ***************************************************************************/

MessageServerFull::MessageServerFull(const std::string& serverName,
		unsigned int serverPort, const MessageFrameworkNode& controllerInfo)
    :   ownerManager_(serverName),
        timeOutMilliSecond_(TIME_OUT_IN_MILISECOND),
        messageDispatcher_(this, this),
        connector_(this, io_service_),
        connectionEstablished_(false)
{
	server_.nodeIP_ = getLocalHostIp(io_service_);
	server_.nodePort_ = serverPort;
	connector_.listen(serverPort);

	controllerNode_.nodeIP_ = getHostIp(io_service_, controllerInfo.nodeIP_);
	controllerNode_.nodePort_ = controllerInfo.nodePort_;
	controllerNode_.nodeName_ = controllerInfo.nodeName_;

	connector_.connect(controllerInfo.nodeIP_, controllerInfo.nodePort_);

	// create new thread for I/O operations
	ioThread_ = new boost::thread(boost::bind(&boost::asio::io_service::run, &io_service_));

	// Added by Wei Cao, 2009-02-20
	// To slove the problem of service registration timeout.
	// waiting until connection to controller is established.
	boost::system_time const timeout = boost::get_system_time()
			+ boost::posix_time::milliseconds(timeOutMilliSecond_);
	boost::mutex::scoped_lock connectionEstablishedLock(
			connectionEstablishedMutex_);
	try {
		while (!connectionEstablished_) {
			DLOG(WARNING)<<"Fail connecting to controller, try again ...";
			connector_.connect(controllerInfo.nodeIP_, controllerInfo.nodePort_);
			if (!connectionEstablishedEvent_.timed_wait(connectionEstablishedLock, timeout))
				throw MessageFrameworkException(SF1_MSGFRK_CONNECTION_TIMEOUT, __LINE__, __FILE__);
		}
	}
	catch(MessageFrameworkException &e) {
        DLOG(ERROR)<<"Catch an exception while connectting to controller : ";
		e.output(DLOG(ERROR));
		exit(1);
	}

	// connect to controller successfully now
}

/******************************************************************************
 Description: Destroy communication resources.
 ******************************************************************************/
MessageServerFull::~MessageServerFull() {
	connector_.shutdown();
	ioThread_->join();
	delete ioThread_;
}

bool MessageServerFull::registerService(const ServiceInfo & serviceInfo) {
	try
	{
		std::map<std::string, ServiceInfo>::iterator it;

		// connection has not been established
		if(!connectionEstablished_)
		return false;

		// Check if the service has been registered
		std::string serviceName = serviceInfo.getServiceName();
		it = availableServiceList_.find(serviceName);
		if(it != availableServiceList_.end())
		{
			// service has been registered
			return true;
		}

		ServiceRegistrationMessage message;
		message.setAgentInfo(agentInfo_);
		message.setServiceInfo(serviceInfo);

		sendServiceRegistrationRequest(message);

		boost::system_time const timeout = boost::get_system_time() +
		boost::posix_time::milliseconds(timeOutMilliSecond_);
		boost::mutex::scoped_lock lock(serviceRegistrationReplyMutex_);

		// Modified by Wei Cao, 2009-02-20
		// To slove the problem of service registration timeout
		// Remove serviceRegistrationWaitingQueue_
		// When wake up from a condition variable, you should check whether
		// the condition fulfills, if not, go on waiting.
		while(serviceRegistrationResult_.find(serviceName) ==
				serviceRegistrationResult_.end())
		{
			if(!serviceRegistrationReplyAccessor_.timed_wait(lock, timeout))
			{
				DLOG(ERROR) << "[Server: " << getName() << "]Timed out!!! Connection is going to be closed";
				return false;
			}
		}

		std::map<std::string, bool>::const_iterator iter =
		serviceRegistrationResult_.find(serviceName);

		if(iter != serviceRegistrationResult_.end())
		{
			// The service has been successfully registered, the service
			// is added to the available service list
			availableServiceList_.insert(std::pair<std::string, ServiceInfo>(serviceName, serviceInfo));
			serviceRegistrationResult_.erase(serviceName);

			DLOG(INFO) << "[Server: " << getName() << "] Service " << serviceName
                    << " is successfully registered";

			return true;
		}

		// it's strange to reach here
		throw MessageFrameworkException(SF1_MSGFRK_UNKNOWN_ERROR, __LINE__, __FILE__);
	}
	catch(MessageFrameworkException& e)
	{
		e.output(DLOG(ERROR));
	}
	catch(boost::system::error_code& e)
	{
		DLOG(ERROR) << e;
	}
	catch(std::exception& e)
	{
		DLOG(ERROR) << e.what();
	}

	return false;
}

/*********************************************************************************
 Output:
 requestList - the list of waiting service requests
 Return:
 true - the list is retrieved successfully
 Description: This function copies all requests to the parameter
 requestList. Then, it deletes the requests in the internal queue.
 *********************************************************************************/
bool MessageServerFull::getServiceRequestList(
		std::vector<ServiceRequestInfoPtr>& requestList) {
	try
	{
		if(requestList.size()> 0)
		requestList.clear();
		// connection has not been established
		if(!connectionEstablished_)
		return false;

		/* >>>>>>>>>>>
		 * TODO: NEED TO DECIDE WHETHER THERE SHOULD BE A TIMEOUT FOR THE RESULT    @by MyungHyun - 2009-01-29
		 */
		// otherwise, wait for new request
		//newRequestEvent_.wait(requestQueueLock);

		boost::system_time const timeout = boost::get_system_time()
		+ boost::posix_time::milliseconds(500); //waits 0.5 secs

		boost::mutex::scoped_lock requestQueueLock(requestQueueMutex_);
		while( requestQueue_.empty() )
		newRequestEvent_.wait(requestQueueLock);
		//newRequestEvent_.timed_wait(requestQueueLock, timeout);
		//<<<<<<<<<<<<<<

		while(!requestQueue_.empty())
		{
			requestList.push_back(requestQueue_.front() );
			requestQueue_.pop();
		}
		return true;
	}
	catch(boost::system::error_code& e)
	{
		std::cerr << e << std::endl;
	}
	catch(std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}

	return false;
}

/**
 * @brief This function answers a service request by putting the ServiceResult to the
 * requester of the service.
 * @param
 *	serviceRequest - information about the service request (service name, requester, parameter list)
 * @param
 *	result - the result of the service
 * @return
 *	true - the service result has been successfully put to the requester
 */
bool MessageServerFull::putResultOfService(
		const ServiceRequestInfoPtr& serviceRequestInfo,
		const ServiceResultPtr& result) {

	DLOG(INFO) << "[Server: " << getName() << "] start to send result of "
			<< serviceRequestInfo->getServiceName();

	// connection has not been established
	if (!connectionEstablished_)
		return false;

	sendResultOfService(serviceRequestInfo->getRequester(), result);

	DLOG(INFO) << "[Server: " << getName()
			<< "] Successfull to send result of "
			<< serviceRequestInfo->getServiceName();

	return true;
}

bool MessageServerFull::putResultOfService(const ServiceResultPtr& result) {

	DLOG(INFO) << "[Server: " << getName() << "] start to send result of "
			<< result->getServiceName();

	// connection has not been established
	if (!connectionEstablished_)
		return false;
	sendResultOfService(result->getRequester(), result);
	return true;
}

/*********************************************************************************
 Description: This function answers a service request by putting the ServiceResult
 to the requester of the service.
 *********************************************************************************/
void MessageServerFull::sendResultOfService(
		const MessageFrameworkNode& requester, const ServiceResultPtr& result) {
	DLOG(INFO)<< "============= MessageServer::sendResultOfService ===========";
	messageDispatcher_.sendDataToLowerLayer1(SERVICE_RESULT_MSG, result,
			requester);

}

/******************************************************************************
 Description: This function put the request for a service result in the waiting list.
 ******************************************************************************/
/*void MessageServerFull::receiveServiceRequest(const MessageFrameworkNode& requester,
 unsigned int requestId,
 const std::string& serviceName,
 const std::vector<boost::shared_ptr<VariantType> >& data)*/

void MessageServerFull::receiveServiceRequest(
		const MessageFrameworkNode& requester,
		ServiceRequestInfoPtr & requestInfo) {

	DLOG(INFO) << "[Server: " << getName()
			<< "] Receive request to retrieve result of service "
			<< requestInfo->getServiceName() << "[requestId = "
			<< requestInfo->getRequestId() << "]";

	try
	{
		// if there is only one request and its minor id is one
		//if( requestInfo->getMinorId() == 0 )
		{
			requestInfo->setRequester(requester);
			//	std::cout<<"receiveServiceRequest : after set Requester"<<endl;
			//	requestInfo->display();

			boost::mutex::scoped_lock requestQueueLock(requestQueueMutex_);
			// add to the requestQueue_
			requestQueue_.push(requestInfo);

			requestQueueLock.unlock();
			newRequestEvent_.notify_all();
			return;
		}
	}
	catch (boost::system::error_code& e)
	{
		std::cerr << e << std::endl;
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: "<< e.what() << std::endl;
	}

}

/**
 * @brief This function sends request to ServiceRegistrationServer
 * in order to register a service
 * @param
 *	serviceName - The service name
 * @param
 *  parameterList - The list of parameter types of the service
 * @param
 *	permissionFlag - this flags tell how to serve the service (through Message Controller or
 *				directly at MessageServerFull)
 */
void MessageServerFull::sendServiceRegistrationRequest(
		ServiceRegistrationMessage& message) {
	message.setServer(server_);
	message.setRequester(ownerManager_);
	messageDispatcher_.sendDataToLowerLayer(SERVICE_REGISTRATION_REQUEST_MSG,
			message, controllerNode_);
}

/*********************************************************************************
 Description: This function set the reply of the recent request of service registration
 Input:
 registrationSuccess - true if the registration is successful
 *********************************************************************************/
void MessageServerFull::receiveServiceRegistrationReply(
		const std::string& serviceName, bool registrationSuccess) {
	try
	{
		DLOG(INFO)<<"receiveServiceRegistrationReply...";
		{
			boost::mutex::scoped_lock serviceRegistrationReplyLock(
					serviceRegistrationReplyMutex_);
			serviceRegistrationResult_.insert(
					std::pair<std::string, bool>(serviceName, registrationSuccess));
		}
		serviceRegistrationReplyAccessor_.notify_all();
	}
	catch(boost::system::error_code& e)
	{
		std::cerr << e << std::endl;
	}
	catch(std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}
}

/**
 * @brief This function create a new AsyncStream that is based on tcp::socket
 */
AsyncStream* MessageServerFull::createAsyncStream(
		boost::shared_ptr<tcp::socket> sock) {
	tcp::endpoint endpoint = sock->remote_endpoint();

	DLOG(INFO) << "Remote IP = " << endpoint.address().to_string() << ", port = " << endpoint.port() << std::endl;

	if (!connectionEstablished_) {
		{
			boost::mutex::scoped_lock connectionEstablishedLock(
					connectionEstablishedMutex_);
			connectionEstablished_ = true;
		}
		connectionEstablishedEvent_.notify_all();
	}

	return new AsyncStream(&messageDispatcher_, sock);
}

}// end of namespace messageframework

