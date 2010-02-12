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
#include <net/message-framework/ServiceRegistrationMessage.h>
#include <net/message-framework/ServiceResultServer.h>
#include <net/message-framework/ServiceResultRequester.h>
#include <net/message-framework/PermissionServer.h>
#include <net/message-framework/ServiceRegistrationServer.h>
#include <net/message-framework/AsyncConnector.h>
#include <net/message-framework/MessageDispatcher.h>
#include <net/message-framework/ClientIdServer.h>

#include <sdb/SequentialDB.h>

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
	protected ServiceRegistrationServer, protected ClientIdServer {
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

	/**
	 * @brief The desstructor
	 */
	~MessageControllerFull();

	/**
	 * @brief Run controller until shutdown is called
	 */
    void run(void);

	/**
	 * @brief Shutdown controller.
	 */
	void shutdown(void);

	/**
	 * @brief get Name of the manager who owns this Controller instance
	 */
	inline const std::string& getName() const {
		return ownerManagerName_;
	}

protected:


	/************ Interfaces of Work Thread****************************/
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

	/**
	 * @brief This function create a new AsyncStream that is based on tcp::socket
	 */
	AsyncStream* createAsyncStream(boost::shared_ptr<tcp::socket> sock);

	friend class AsyncConnector;

private:

	/**
	 * @brief Name of the manager that uses MessageControllerFull
	 */
	std::string ownerManagerName_;

	/**
	 * @brief Sequential number used to generate client id
	 */
	int nxtClientId_;

    /**
     * @brief Lock that protects accesses to nxtClientId_
     */
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
	izenelib::sdb::ordered_sdb<std::string, ServicePermissionInfo>  availableServiceList_;

	/**
	 * @brief mutex to allow exclusively access to availableServiceList_
	 */
	boost::mutex availableServiceListMutex_;

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
	 * @brief This variables receives data from peer and sends data to the peer
	 */
	MessageDispatcher messageDispatcher_;

	/**
	 * @brief queue for I/O operations
	 */
	boost::asio::io_service io_service_;
//
//	/**
//	 * @brief Connect to servers
//	 */
//	AsyncConnector asyncConnector_;

	/**
	 * @brief Manage all connections
	 */
    AsyncStreamManager asyncStreamManager_;

	/**
	 * @brief Listen at the controller port
	 */
	AsyncAcceptor asyncAcceptor_;

	/**
	 * @brief thread for processing service registration
	 */
	boost::thread* workThread1_;

	/**
	 * @brief thread for answering service permission
	 */
	boost::thread* workThread2_;

	/**
	 * @brief indicate whether controller is working;
	 */
    bool stop_;
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

	static MessageControllerFull& instance() {
	    if(messageController_ == NULL)
            messageController_ = new MessageControllerFull(
                controllerName_, controllerPort_);
		return *messageController_;
	}

	static void destroy() {
        delete messageController_;
        messageController_ = NULL;
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

	/**
	 * @brief singleton
	 */
	static MessageControllerFull* messageController_;
};

}
// end of messageframework
#endif  //#if !defined(_MESSAGECONTROLLER_FULL_H)
