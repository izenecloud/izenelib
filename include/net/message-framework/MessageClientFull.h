///
///  @file  MessageClientFull.h
///  @date  8/5/2008
///  @author  TuanQuang Nguyen
///  @brief This file defines MessageClientFull class


#if !defined(_MESSAGECLIENT_FULL_H)
#define _MESSAGECLIENT_FULL_H


/**************** Include module header files *******************/
#include <net/message-framework/ServicePermissionInfo.h>
#include <net/message-framework/MessageFrameworkNode.h>
#include <net/message-framework/ServiceResultRequester.h>
#include <net/message-framework/PermissionRequester.h>
#include <net/message-framework/MessageDispatcher.h>
#include <net/message-framework/AsyncConnector.h>
#include <net/message-framework/ClientIdRequester.h>
#include <net/message-framework/Semaphore.h>


/**************** Include boost header files *******************/
#include <boost/asio.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/noncopyable.hpp>

/**************** Include std header files *******************/
#include <set>
#include <map>
#include <vector>
#include <string>

/**************** Include sf1lib header files *******************/

#define MF_FULL_REQUEST_ID_CLIENT_ID_SHIFT 20
#define MF_FULL_MAX_SEQUENTIAL_NUMBER (1 << MF_FULL_REQUEST_ID_CLIENT_ID_SHIFT)
#define MF_FULL_MASK_SEQUENTIAL_NUMBER ( MF_FULL_MAX_SEQUENTIAL_NUMBER - 1)

namespace messageframework
{

	/**
	 * @brief MessageClientFull is a class to send request of the manager, and to get
	 * result from MessageServer or from MessageController. A Manager that needs
	 * result of other Managers in different process should use the MessageClientFull.
	 */
	class MessageClientFull : protected ServiceResultRequester, protected PermissionRequester,
		protected ClientIdRequester, protected AsyncStreamFactory
	{
	public:
		/**
		 * @brief construct a MessageClientFull given the client name and
		 * information of controller. The MessageClientFull will connects to controller.
		 * @param
		 * clientName - name of client
		 * @param
		 * controllerInfo - information of controller
		 */
		MessageClientFull(const std::string& clientName,
					const MessageFrameworkNode& controllerInfo);

		/**
		 * @brief destructor, destroy variables it if is neccessary
		 */
		~MessageClientFull();

		/**
		 * @brief This function gets an ID from MessageController. This ID
		 * will be used combined with a sequential number to generate requestIds.
		 * @param
		 * clientId - controller assigned ID
		 * @return
		 * true - ID is got successfully
		 * @return
		 * false - MessageController is busy.
		 */
		//bool getClientId(int& clientId);	   

		/**
		 * @brief This function gets a permission of the given service from
		 * MessageController. If MessageController is busy,
		 * this function returns false immediately. Then, the MessageClientFull
		 * has to call this function again
		 * @param
		 * serviceName - the service name
		 * @param
		 * servicePermissionInfo - permission information of the service. If
		 * the service is available, it contains information of the server.
		 * @return
		 * true - Available to request service
		 * @return
		 * false - the MessageController is busy
		 */
		
		//bool getPermissionOfService(const std::string& serviceName,
		//					ServicePermissionInfo& servicePermissionInfo);
		
		bool getHostsOfService(const std::string& serviceName,
				std::map<std::string, MessageFrameworkNode>& servers);
		
		
		void flushPermissionCache(const std::string& serviceName);
			
		
		/**
		 * @brief This function puts the request of the manager to the MessageClientFull.
		 * The MessageClientFull will sends the request to either MessageController or
		 * MessageServerFull.
		 * @param
		 * servicePermissionInfo - it contains information of service name and the server
		 * @param
		 * serviceRequestInfo - information about request service. It contains the
		 * service name and its parameter values.
		 * @return
		 * true - if the receiver successfully receives the request
		 */
		bool putServiceRequest(const MessageFrameworkNode& server,
							ServiceRequestInfoPtr& serviceRequestInfo, bool withResult = true);

		/**
		 * @brief This function puts a set of requests of the same manager to the
		 * MessageClientFull. The MessageClientFull will sends the request to either
		 * MessageController or MessageServerFull.
		 * @param
		 * servicePermissionInfo - it contains information of service name and the server
		 * @param
		 * serviceRequestInfos - a set of information about request services, each contains
		 * the service name and its parameter values.
		 * @return
		 * true - if the receiver successfully receives these requests
		 */
		bool putServiceRequest(const MessageFrameworkNode& server,
							std::vector<ServiceRequestInfoPtr>& serviceRequestInfos, bool withResult = true);

		/**
		 * @brief This function gets a result of the service that have been requested.
		 * The service is requested through function putServiceRequest(...).
		 * When the result is not ready, it returns false immediately.
		 * @param
		 * serviceRequestInfo - it contains information of request Id, service name
		 * @param
		 * serviceResult - information about result. I contains the result of the service.
		 * @return
		 * true - Result is ready.
		 * @return
		 * false - result is not ready.
		 */
		bool getResultOfService(const ServiceRequestInfoPtr& serviceRequestInfo,
						ServiceResultPtr& serviceResult);

		/**
		 * @brief This function gets a set of results of the service
		 * that have been requested. The service is requested through function
		 * putServiceRequest(..) When the result is not ready, it returns false immediately.
		 * @param
		 * serviceRequestInfos - a set of service request informations
		 * @param
		 * serviceResults - a set of service results
		 * @return
		 * true - Result is ready.
		 * @return
		 * false - result is not ready.
		 */
		bool getResultOfService(const std::vector<ServiceRequestInfoPtr> & serviceRequestInfo,
						std::vector<ServiceResultPtr> & serviceResults);

		/** @brief for profiling - Wei Cao */
		inline MessageDispatcher& getMessageDispatcher()
		{
			return messageDispatcher_;
		}

		/** Set number of requests be processed at a time **/
		inline int getBatchProcessedRequestNumber()
		{
			return batchProcessedRequestNumber_;
		}

		/** Get number of requests be processed at a time **/
		inline void setBatchProcessedRequestNumber( int batchProcessedRequestNumber )
		{
			batchProcessedRequestNumber_ = batchProcessedRequestNumber;
		}

	protected:

		/*** Interfaces of ServiceResultRequester ***/

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
				const std::vector<boost::shared_ptr<VariantType> >& data, const MessageFrameworkNode& server) ;*/


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
	/*	void receiveResultOfService(unsigned int requestId,
						const std::string& serviceName,
						const std::vector<boost::shared_ptr<VariantType> >& data);*/


		void sendServiceRequest(const ServiceRequestInfoPtr& requestInfo, const MessageFrameworkNode& server) ;


		void receiveResultOfService(const ServiceResultPtr& result);

		/*** End of Interfaces of ServiceResultRequester ***/

		/*** Interfaces of ClientIdRequester ***/
		/**
		 * @brief This function sends a request to get client id
		 */
		//void sendClientIdRequest();

		/**
		 * @brief This function saves client id
		 * @param
		 * clientId - controller assigned client id
		 */
		//void receiveClientIdResult(const int& clientId);

		/*** End of Interfaces of ServiceResultRequester ***/

		/*** Interfaces of PermissionRequester ***/
		/**
 		 * @brief This function sends a request to get Permission of a service
 		 * @param serviceName the service name
 		 */
		void sendPermissionOfServiceRequest(
								const std::string& serviceName);

		/**
 		 * @brief This function push PermissionOfService to the internal list
 		 * @param
 		 * servicePermissionInfo - the information of the PermissionOfService
 		 */
		void receivePermissionOfServiceResult(
							const  ServicePermissionInfo& servicePermissionInfo);
		/*** End of Interfaces of PermissionRequester ***/

		const std::string& getName(){return ownerManager_;}

		/**
		 * @brief This function generate request id
		 */
		const unsigned int generateRequestId();

		/**
		 * @brief This function check connection to remote node, if connection
		 * havn't been established, this function will try to prepare the connection.
		 * @param
		 * node - remote node
		 * @return
		 * false - if the connection to remote node cannot be established in the given time.
		 */
		bool prepareConnection(const MessageFrameworkNode& node);

		/**
 	     * @brief This function create a new AsyncStream that is based on tcp::socket
 		 */
		AsyncStream* createAsyncStream(boost::shared_ptr<tcp::socket> sock);

		friend class AsyncConnector;

	private:
		bool checkAgentInfo_(ServicePermissionInfo& permissionInfo);
		bool checkPermissionInfo_(ServicePermissionInfo& permissionInfo);
		
		/**
		 * @brief The request list, it contains the result of the service if the result
		 * has come
		 */
		std::map<unsigned int, std::vector<boost::shared_ptr<ServiceResult> > > serviceResultTable_;
		std::map<unsigned int, int > serviceResultCountTable_;

		/**
 		 * @brief This mutex variable is used to exclusively access to serviceResultTable_
 		 */
		boost::mutex serviceResultMutex_;

		/**
		 * @brief The semaphore table, it contains all semaphores that currently being
		 * used by each request.
		 */
		std::map<unsigned int, boost::shared_ptr<semaphore_type> > semaphoreTable_;
		boost::mutex semaphoreMutex_;

		/**
		 * @brief Those request ids are waiting results from remote server.
		 */
		std::set<unsigned int> uncompletedRequestSet_;
		boost::mutex uncompletedRequestMutex_;

		/**
		 * @brief Those request ids are finished and waiting to be got away.
		 */
		std::set<unsigned int> completedRequestSet_;
		boost::mutex completedRequestMutex_;

		/**
 		 * @brief This variable stores the PermissionServiceInfo recently receceived
 		 * from MessageController. This variable is considered as cache of
 		 * accepted permission.
 		 */
		std::map<std::string, ServicePermissionInfo> acceptedPermissionList_;

		/**
 		 * @brief This variable is used for thread synchronization between
 		 * data-sending and data-receving threads of PermissionServiceInfo.
 		 */
		boost::condition_variable newPermisionOfServiceEvent_;

		/**
 		 * @brief This mutex variable is used for thread synchrnonization between
 		 * data-sending and data-receving threads of acceptedPermissionList_
 		 */
		boost::mutex acceptedPermissionMutex_;

		/**
 		 * @brief Name of the manager that uses MessageClientFull
 		 */
		std::string ownerManager_;

		/**
		 * @brief Controller assigned Client ID
		 */
		int clientId_;
		boost::mutex clientIdMutex_;
		boost::condition_variable clientIdEvent_;

		/**
		 * @brief Sequential number used together with clientId_ to generate request ID
		 */
		int nxtSequentialNumber_;
		boost::mutex nxtSequentialNumberMutex_;

		// information of the server of ServicePermision
		MessageFrameworkNode servicePermisionServer_;


		/**
 		 * @brief This variables receives data from peer and sends data to the peer
 		 */
		MessageDispatcher messageDispatcher_;

		/**
 		 * @brief The maximum time in milliseconds to wait to process data.
 		 * If no data arrives during this period, this system considers it as error
 		 */
		unsigned int timeOutMilliSecond_;

		/**
 		 * @brief queue of I/O operations
 		 */
		boost::asio::io_service io_service_;

		/**
 		 * @brief connector to connect to controller
 		 */
		AsyncConnector asyncConnector_;

		/**
 		 * @brief thread for I/O operations
 		 */
		boost::thread* ioThread_;

		/**
		 * @brief number of requests be processed at a time
		 */
		int batchProcessedRequestNumber_;

		/**
 		 * @brief the controller node
 		 */
		MessageFrameworkNode controllerNode_;
		bool connectionToControllerEstablished_;
		boost::mutex connectionToControllerEstablishedMutex_;
		boost::condition_variable connectionToControllerEvent_;

		bool connectionToServerEstablished_;
		boost::mutex connectionToServerEstablishedMutex_;
		boost::condition_variable connectedToServerEvent_;

		// time for context switching
		clock_t contextSwitchTick1, contextSwitchTick2;
		struct timeval contextSwitchVal1, contextSwitchVal2;

		// time for waiting data from socket
		clock_t socketDataWaitingTick1, socketDataWaitingTick2;
		struct timeval socketDataWaitingVal1, socketDataWaitingVal2;
	};
 // typedef boost::shard_ptr<MessageClientFull> MessageClientFullPtr;

}// end of namespace messageframework
#endif  //#if !defined(_MESSAGECLIENT_FULL_H)
