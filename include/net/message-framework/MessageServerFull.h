///
///  @file : MessageServerFull.h
///  @date : 8/5/2008
///  @author : TuanQuang Nguyen
///

#if !defined(_MESSAGESERVER_FULL_H)
#define _MESSAGESERVER_FULL_H

/**************** Include module header files *******************/
#include <net/message-framework/MessageFrameworkNode.h>
#include <net/message-framework/ServiceMessage.h>
#include <net/message-framework/ServiceResultServer.h>
#include <net/message-framework/ServiceRegistrationRequester.h>
#include <net/message-framework/MessageDispatcher.h>
#include <net/message-framework/AsyncConnector.h>
#include <net/message-framework/ThreadPool.h>

/**************** Include boost header files *******************/
#include <boost/thread/condition_variable.hpp>
#include <boost/thread.hpp>

/**************** Include std header files *******************/
#include <queue>
#include <set>
#include <string>
#include <vector>


namespace messageframework
{
	/**
	 * @brief MessageServerFull is a class to serve the result of the given service request. Before
	 * serving, the MessageServerFull registers its services to MessageController. Manager which
	 * must provide result to the other Manager in different process should use MessageServerFull.
	 */
	class MessageServerFull : protected ServiceResultServer,
                            protected ServiceRegistrationRequester,
                            protected AsyncStreamFactory
	{
	public:
		/**
		 * @brief Construct a MessageServerFull with paramters. It initializes
		 * resources for communication and connect to MessageController.
		 * @param
		 * serverName - the name of server
		 * @param
		 * serverPort - port of service that serves results of services
		 * @param
		 * controllerInfo - information of controller (IP and port)
		 */
		MessageServerFull(const std::string& serverName, unsigned int serverPort,
					const MessageFrameworkNode& controllerInfo);

		/**
		 * @brief Destroy variables if it is neccessary. It destroys communication resources.
		 */
		~MessageServerFull();

		/**
		 * @brief This function register a service of the current MessageServerFull. It will
		 * submit the registration information of the service to the MessageController.
		 * @param
		 *	serviceName - The service name
		 * @param
		 *  parameterList - The list of parameter types of the service
		 * @param
		 *	permissionFlag - this flags tell how to serve the service (through Message Controller or
		 *				directly at MessageServerFull)
		 * @return
		 *	true - the service has been successfully registered.
		 */
		bool registerService(const std::string& serviceName,
                         const std::vector<ServiceParameterType>& parameterList,
                         const PermissionFlag& permissionFlag,
                         const ServiceResultFlag serviceResultFlag = SERVICE_WITH_RESULT );

		/**
		 * @brief   This function register a service of the current MessageServerFull. It will
		 *          submit the registration information of the service to the MessageController.
		 *
		 * @param serviceInfo   The service info which holds the information of the service to be registered
		 * @return  True, if the service has been successfully registered.
		 * @by MyungHyun - Feb 10, 2009
		 */
		bool registerService( const ServiceInfo & serviceInfo );

		/**
		 * @brief This function gets all the list of waiting service requests. It deletes the waiting
		 * service requests in the queue.
		 * @param
		 *	requestList - the list of waiting service requests
		 * @return
		 *	true - the list is retrieved successfully
		 */
		bool getServiceRequestList(std::vector<ServiceRequestInfoPtr>& requestList);

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
		bool putResultOfService(const ServiceRequestInfoPtr& serviceRequestInfo,
                            const ServiceResultPtr& result);

		bool putResultOfService(const ServiceResultPtr& result);

		/** @brief for profiling - Wei Cao */
		inline MessageDispatcher& getMessageDispatcher() { return messageDispatcher_; }

	protected:
		const MessageFrameworkNode& getServerInfo(){return server_;}


		/************** interfaces of ServiceRegistrationRequester **************/
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
		void sendServiceRegistrationRequest(const std::string& serviceName,
				const std::vector<ServiceParameterType>& parameterList,
				const PermissionFlag& permissionFlag,
        const ServiceResultFlag serviceResultFlag = SERVICE_WITH_RESULT);

		/**
 		 * @brief This function set the reply of the recent request of service registration
 		 * @param
 		 * serviceName - the service name to register
 		 * @param
 		 * registrationSuccess - true if the registration is successful
 		 */
		void receiveServiceRegistrationReply(const std::string& serviceName, bool registrationSuccess);

		/************** End of interfaces of ServiceRegistrationRequester **************/

		/************** interfaces of ServiceResultServer **************/

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

	   void receiveServiceRequest(const MessageFrameworkNode& requester,
						ServiceRequestInfoPtr & requestInfo) ;
		
		void sendResultOfService(const MessageFrameworkNode& requester,				
				const ServiceResultPtr& result) ;


		/************** End of interfaces of ServiceResultServer **************/

    /**
		 * @brief get Name of the server
		 */
    const std::string& getName() const { return ownerManager_; }

		/**
 	     * @brief This function create a new AsyncStream that is based on tcp::socket
 		 */
		AsyncStream* createAsyncStream(boost::shared_ptr<tcp::socket> sock);

		friend class AsyncConnector;

	private:
		/**
 		 * @brief Name of the manager that uses MessageServerFull
 		 */
		std::string ownerManager_;

		/**
 		 * @brief List of available services. It contains all the registered services.
 		 */
		std::map<std::string, ServiceInfo> availableServiceList_;

		/**
		 * @brief The queue contains requests that are waiting for being served
		 */
		std::queue<boost::shared_ptr<ServiceRequestInfo> > requestQueue_;

		/**
		 * @brief This Map contains a list of results with the same request id but
		 * differenet minor id
		 */
		std::map<unsigned int, std::vector<boost::shared_ptr<ServiceResult> > > resultCacheMap_;

		std::map<unsigned int, int> expectedResultNumberMap_;

		std::map<unsigned int, int> actualResultCountMap_;

		boost::mutex resultCacheMutex_;

		/**
 		 * @brief the mutex to allow exclusive access to requestQueue_
 		 */
		boost::mutex requestQueueMutex_;
		boost::condition_variable newRequestEvent_;

		/**
 		 * @brief IP address and port number of the server
 		 */
		MessageFrameworkNode server_;

		/**
 		 * @brief This mutex variable is used for thread synchrnonization between
 		 * data-sending and data-receving threads of service registration information.
 		 */
		boost::mutex serviceRegistrationReplyMutex_;

		/**
 		 * @brief This variable is used to gain access to the result of the
 		 * service registration. It is used for thread synchronization between
 		 * data-sending and data-receving threads of service registration information.
 		 */
		boost::condition_variable serviceRegistrationReplyAccessor_;

		/**
		 * @brief This variable store result of service registration procedure.
		 */
		std::map<std::string, bool> serviceRegistrationResult_;

		/**
 		 * @brief Information of the server to register services
 		 */
		MessageFrameworkNode serviceRegistrationServer_;

		/**
 		 * @brief The maximum time in milliseconds to wait to process data.
 		 * If no data arrives during this period, this system considers it as error
 		 */
		unsigned int timeOutMilliSecond_;

		/**
 	  	 * @brief listening port to serve results of services
 	  	 */
		unsigned int listeningPort_;

		/**
 		 * @brief MessageDispatcher plays an intermediary role between network layer
 		 * and application layer
 		 */
		MessageDispatcher messageDispatcher_;

		/**
 		 * @brief Queue for I/O operations
 		 */
		boost::asio::io_service io_service_;

		/**
 		 * @brief connection with controller
 		 */
		AsyncConnector connector_;

		/**
 		 * @brief thread for I/O operations
 		 */
		boost::thread* ioThread_;

		/**
 	  	 * @brief local end point of the connection to MessageController
 	  	 */
		MessageFrameworkNode controllerNode_;
		bool connectionEstablished_;
		boost::mutex connectionEstablishedMutex_;
		boost::condition_variable connectionEstablishedEvent_;

		clock_t tick1, tick2;
		struct timeval val1, val2;
	};
	//typedef boost::shard_ptr<MessageServerFull> MessageServerFullPtr;
	
}// end of messageframework
#endif  //#if !defined(_MESSAGESERVER_FULL_H)
