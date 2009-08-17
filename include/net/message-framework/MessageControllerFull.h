///
///  @file : MessageControllerFull.h
///  @date : 8/5/2008
///  @author : TuanQuang Nguyen
///  @brief This file defines PermissionOfServiceHandler class and MessageControllerFull class
///


#if !defined(_MESSAGECONTROLLER_FULL_H)
#define _MESSAGECONTROLLER_FULL_H

/**************** Include module header files *******************/
#include <net/message-framework/ServiceInfo.h>
#include <net/message-framework/ServicePermissionInfo.h>
//#include <message-framework/ServiceResult.h>
//#include <message-framework/ServiceRequestInfo.h>
//#include <net/message-framework/PermissionOfServiceMessage.h>
#include <net/message-framework/ServiceRegistrationMessage.h>
#include <net/message-framework/ServiceResultServer.h>
#include <net/message-framework/ServiceResultRequester.h>
#include <net/message-framework/PermissionServer.h>
#include <net/message-framework/ServiceRegistrationServer.h>
#include <net/message-framework/AsyncConnector.h>
#include <net/message-framework/MessageDispatcher.h>
#include <net/message-framework/ClientIdServer.h>

/**************** Include wiselib header files *******************/
//#include <wiselib/hash/LinearHashTable.h>

/**************** Include boost header files *******************/
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

/**************** Include std header files *******************/
#include <queue>

namespace messageframework {

#define  MF_FULL_MAX_CLIENT_ID 255

/**
 * @brief MessageControllerFull controls the messages between MessageClients
 * and MessageServers. It also contains the list of pairs of (service,
 * MessageServer) to to determine the destination of message.
 */

class MessageControllerFull : protected ServiceResultServer,
	protected ServiceResultRequester, protected PermissionServer,
	protected ServiceRegistrationServer, protected ClientIdServer,
	protected AsyncStreamFactory {
public:
	/**
	 * @brief The construct a MessageControllerFull with paramters
	 * @param
	 * controllerName - name of controller
	 * @param
	 * servicePort - port of controller
	 */
	MessageControllerFull(const std::string& controllerName,
			unsigned int servicePort);

	// MessageControllerFull(const std::string& controllerName, const ControllerInfo& controllerInfo);
	//
	/**
	 * @brief The desstructor
	 */
	~MessageControllerFull();

	// void start(unsigned int Port = 7777);

	/**
	 * @brief This function processes all the waiting service registration requests.
	 * It registers all the services from MessageServer (IP address, port number,
	 * a list of service which can be provided by the MessageServer).
	 */
	void processServiceRegistrationRequest(void);

	/**
	 * @brief This function processes all the waiting service permission requests.
	 * It determines whether it is available to process the service and issues
	 * permission to the MessageClient. If the service needs to use the direct
	 * connection, MessageControllerFull sends the permission to the MessageServer.
	 */
	void processServicePermissionRequest(void);

	/**
	 * @brief This function processes all waiting service requests from MessageClient.
	 * It will deliver these messages MessageServers.
	 */
	void processServiceRequestFromClient(void){;}

	/**
	 * @brief This function processes all the coming results from MessageServer. It will
	 * deliver the results to the MessageClient.
	 */
	void processServiceResultFromServer(void){;}

protected:

	// old name: getPermissionOfService
	// new name: getServiceInfo
	// The function does not match the implementation, it returns only ServiceInfo
	/**
	 * @brief This function retrieves a information of service. It returns false if the
	 * service is not found.
	 * @param
	 * serviceName - the service name
	 * @param
	 * serviceInfo - the information of service
	 */
	//bool
	//		getServiceInfo(const std::string& serviceName,
	//				ServiceInfo& serviceInfo);

	/**
	 * @brief This function forward the request to the server and
	 * put the request for a service result in the waiting list.
	 * @param requestInfo informationi of the request
	 */
	//void forwardServiceRequest(const ServiceRequestInfo& requestInfo);

	/************ Interfaces of Service Registration Server ****************/
	/**
	 * @brief This function inserts a ServiceInfo to the serviceRegistrationRequestQueue_.
	 * @param
	 * localEndPoint the end point that connects to the ServiceRegistrationRequester
	 * @param
	 * serviceInfo - data to add
	 */
	void receiveServiceRegistrationRequest(
			const MessageFrameworkNode& localEndPoint,
			const ServiceRegistrationMessage& serviceInfo);

	/**
	 * @brief This function replies to a registration request from ServiceRegistrationRequester. It
	 * will sends registration result to the ServiceRegistrationRequester.
	 * @param localEndPoint the end point that connects to the ServiceRegistrationClient.
	 * @param serviceInfo the service information to register
	 * @param success the registration result
	 */
	void sendServiceRegistrationResult(
			const MessageFrameworkNode& localEndPoint,
			const ServiceInfo& serviceInfo, bool success);

	/************ End of Interfaces of Service Registration Server ****************/

	/************ Interfaces of Permission Server ****************/
	// old name: putPermissionOfService
	// new name: putPermissionOfServiceRequest
	// The function inherits from PermissionServer

	/**
	 * @brief This function put a request to get  permission of service into the
	 * waiting queue. The requests in the waiting queue will be processed in function
	 * processServicePermissionRequest().
	 * @param
	 * requester node that requests a permission
	 * @param
	 * serviceName the service name to get permission
	 */
	void receivePermissionOfServiceRequest(
			const MessageFrameworkNode& requester,
			const std::string& serviceName);

	/**
	 * @brief This function replies to a request from PermissionRequester
	 * @param requester node that requests a permission
	 * @param permission the permission
	 */
	void sendPermissionOfServiceResult(const MessageFrameworkNode& requester,
			const ServicePermissionInfo& permission);

	/************ End of Interfaces of Permission Server ****************/

	/************ Interfaces of ClientIdServer *****************/

	/**
	 * @brief When a new request of get client id comes, this function
	 *    is called in order to notify the server
	 * @param requester - node that requests a client id
	 */
	void receiveClientIdRequest(const MessageFrameworkNode& requester);

	/**
	 * @brief This function replies to a request from ClientIdRequester
	 * @param requester - node that requests for a permission
	 * @param clientId - assigned client id
	 */
	void sendClientIdResult(const MessageFrameworkNode& requester,
			const int& clientId);

	/************ End of Interfaces of ClientIdServer ****************/

	/************ Interfaces of Service Result Requester ****************/

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
	/*void sendServiceRequest(unsigned requestId,
	 const std::string& serviceName,
	 const std::vector<boost::shared_ptr<VariantType> >& data,
	 const MessageFrameworkNode& server);*/

	/**
	 * @brief This function put the result of a serivce. The service result
	 * has been requested before.
	 * @param
	 * requestId - id of the request
	 * @param
	 * serviceName - the name of service
	 * @param
	 * data - result of the service
	 */
	/*void receiveResultOfService(unsigned int requestId,
	 const std::string& serviceName,
	 const std::vector<boost::shared_ptr<VariantType> >& data) ;*/
	/************ End of Interfaces of Service Result Requester ****************/

	/************ Interfaces of Service Result Server ****************/

	/**
	 * @brief This function put the request for a service result in the waiting list.
	 * @param requester requester that requests for result
	 * @param requestId the id of the request
	 * @param serviceName the service name
	 * @param data the parameter data of the request
	 */
	/*void receiveServiceRequest(const MessageFrameworkNode& requester,
	 unsigned int requestId,
	 const std::string& serviceName,
	 const std::vector<boost::shared_ptr<VariantType> >& data);*/

	/**
	 * @brief This function replies to a request from ServiceResultRequester. I will sends
	 * ServiceResult to the ServiceResultRequester.
	 * @param
	 * requester - the requester that requests for result
	 * @param
	 * requestInfo - information of the request
	 * @param
	 * result - result of the requested service
	 */
	/*void sendResultOfService(const MessageFrameworkNode& requester,
	 const ServiceRequestInfo& requestInfo,
	 const ServiceResult& result);*/

	/************ End of Interfaces of Service Result Server ****************/

	/**
	 * @brief get Name of the controller
	 */
	const std::string& getName() const {
		return ownerManager_;
	}

	/**
	 * @brief This function create a new AsyncStream that is based on tcp::socket
	 */
	AsyncStream* createAsyncStream(boost::shared_ptr<tcp::socket> sock);

	friend class AsyncConnector;

private:

	/**
	 * @brief Name of the manager that uses MessageControllerFull
	 */
	std::string ownerManager_;

	/**
	 * @brief Sequential number used to generate client id
	 */
	int nxtClientId_;

	boost::mutex nxtClientIdMutex_;

	/**
	 * @brief the queue of waiting service registration requests.
	 */
	std::queue<std::pair<ServiceRegistrationMessage, MessageFrameworkNode> >
			serviceRegistrationRequestQueue_;

	/**
	 * @brief serviceRegistrationRequestQueue_mutex_ is used with serviceRegistrationRequestQueue_Access_
	 * to make sure that only one thread can access to serviceRegistrationRequestQueue_
	 */
	boost::mutex serviceRegistrationRequestQueueMutex_;

	/**
	 * @brief event is signalized when new registration request arrives
	 */
	boost::condition_variable newRegistrationEvent_;

	/**
	 * @brief the queue of waiting service permission requests. It is a queue of pair (service name,
	 * local end point)
	 */
	std::queue<std::pair<std::string, MessageFrameworkNode> >
			servicePermissionRequestQueue_;

	/**
	 * @brief mutex to allow exclusively access to servicePermissionRequestQueue_
	 */
	boost::mutex servicePermissionRequestQueueMutex_;

	/**
	 * @brief event is signalized when new permission request arrives
	 */
	boost::condition_variable newPermissionRequestEvent_;

	/**
	 * @brief the list of availabe service
	 */
	//boost::unordered_map<std::string, ServiceInfo> availableServiceList_;
	boost::unordered_map<std::string, ServicePermissionInfo> availableServiceList_;

	/**
	 * @brief mutex to allow exclusively access to availableServiceList_
	 */
	boost::mutex availableServiceListMutex_;

	/**
	 * @brief the queue of waiting service requests
	 */
	//std::queue<std::pair<MessageFrameworkNode, ServiceRequestInfoPtr> >
	//		serviceRequestQueue_;

	/**
	 * @brief the mutex to allow exclusive access to serviceRequestQueue_
	 */
	//boost::mutex serviceRequestQueueMutex_;

	/**
	 * @brief this variable is used as a signal to a new service request
	 * It is signalized when a new request comes.
	 */
	//boost::condition_variable newServiceRequestEvent_;

	/**
	 * @brief Listening port to serve service result
	 */
	unsigned int servicePort_;

	/**
	 * @brief IP address of MessageControllerFull
	 */
	std::string ipAddress_;

	/**
	 * @brief The maximum time in milliseconds to wait to process data.
	 * If no data arrives during this period, this system considers it as error
	 */
	unsigned int timeOutMilliSecond_;

	/**
	 * @brief This variable store all requests that have been sent to server but have not
	 * been received the result
	 */
	//std::map<unsigned int, MessageFrameworkNode> waitingForResultMessageQueue_;

	/**
	 * @brief This mutext allows exclusive access to waitingForResultMessageQueue_
	 */
	//boost::mutex waitingForResultMessageQueueMutex_;

	/**
	 * @brief This variable is used to notify if the result of service has been
	 * successfully sent.
	 */
	//bool serviceResultReply_;

	/**
	 * @brief This mutex is used to allow exclusively access to  serviceResultReply_
	 */
	//boost::mutex serviceResultReplyMutex_;

	/**
	 * @brief This queue contains results of requested services
	 */
	//std::queue<std::pair<MessageFrameworkNode, ServiceResultPtr> > resultQueue_;

	/**
	 * @brief This mutex is used to allow exclusive access to resultQueue_
	 */
	//boost::mutex resultQueueMutex_;

	/**
	 * @brief This event is signalized when new result of service arrives
	 */
	//boost::condition_variable newServiceResultEvent_;

	/**
	 * @brief This variables receives data from peer and sends data to the peer
	 */
	MessageDispatcher messageDispatcher_;

	/**
	 * @brief queue for I/O operations
	 */
	boost::asio::io_service io_service_;

	/**
	 * @brief This connector is used to listen at the controller port and connect
	 * to servers
	 */
	AsyncConnector asyncConnector_;

	/**
	 * @brief thread for I/O operations
	 */
	boost::thread* ioThread_;
};

/**
 * @brief This class produces singleton of MessageControllerFull
 */
class MessageControllerFullSingleton : private boost::noncopyable {
public:
	static void init(const std::string& controllerName,
			unsigned int controllerPort) {
		controllerName_ = controllerName;
		controllerPort_ = controllerPort;

	}

	static void _init() {
		//call_once
		messageController_.reset(new MessageControllerFull(controllerName_, controllerPort_));
	}

	static MessageControllerFull& instance() {
		boost::call_once(_init, flag);

		return *messageController_;
	}

private:
	MessageControllerFullSingleton() {
	}
	~MessageControllerFullSingleton() {
	}

private:
	/**
	 * @brief name of controller
	 */
	static std::string controllerName_;
	/**
	 * @brief port of controller
	 */
	static unsigned int controllerPort_;

	static boost::scoped_ptr<MessageControllerFull> messageController_;
	static boost::once_flag flag;
};

/*
 boost::scoped_ptr<MessageControllerFull> MessageControllerFullSingleton::messageController_(0);
 boost::once_flag MessageControllerFullSingleton::flag = BOOST_ONCE_INIT;
 std::string MessageControllerFullSingleton::controllerName_("");
 unsigned int MessageControllerFullSingleton::controllerPort_ = 0;
 */

}
// end of messageframework
#endif  //#if !defined(_MESSAGECONTROLLER_FULL_H)
