/**
 */


#ifndef _MESSAGE_CLIENT_LIGHT_H_
#define _MESSAGE_CLIENT_LIGHT_H_

#include <net/message-framework/MessageControllerLight.h>

#include <string>

#define MF_LIGHT_REQUEST_ID_CLIENT_ID_SHIFT 24
#define MF_LIGHT_MAX_SEQUENTIAL_NUMBER (1 << MF_LIGHT_REQUEST_ID_CLIENT_ID_SHIFT)
#define MF_LIGHT_MASK_SEQUENTIAL_NUMBER ( MF_LIGHT_MAX_SEQUENTIAL_NUMBER - 1)

namespace messageframework
{
    class MessageClientLight
    {
        public:

            /**
             * @brief Sets the client name and controller reference
             */
            MessageClientLight( const std::string clientName,
				MessageControllerLight & controller );

			bool getClientId(int& clientId);

            /**
             * @brief This function gets a permission of the given service from
             * MessageController. If MessageController is busy,
             * this function returns false immediately. Then, the MessageClient
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
            bool getPermissionOfService(const std::string& serviceName,
                    ServicePermissionInfo& servicePermissionInfo);

            /**
             * @brief This function puts the request of the manager to the MessageClient.
             * The MessageClient will sends the request to either MessageController or
             * MessageServer.
             * @param
             * servicePermissionInfo - it contains information of service name and the server
             * @param
             * serviceRequestInfo - information about request service. It contains the
             * service name and its parameter values.
             * @return
             * true - if the receiver successfully receives the request
             */
            //TODO: why don't we have only ServiceRequestInfo passed as parameter
            bool putServiceRequest(const ServicePermissionInfo& servicePermissionInfo,
                    ServiceRequestInfoPtr& serviceRequestInfo);

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
			bool putServiceRequest(const ServicePermissionInfo& servicePermissionInfo,
								std::vector<ServiceRequestInfoPtr>& serviceRequestInfos);


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
			bool getResultOfService(const std::vector<ServiceRequestInfoPtr> & serviceRequestInfos,
							std::vector<ServiceResultPtr> & serviceResults);

		protected:

			const unsigned int generateRequestId();

        private:

            std::string clientName_;

            MessageControllerLight & controller_;

			/**
			 * @brief Controller assigned Client ID
			 */
			int clientId_;

			/**
			 * @brief Sequential number used together with clientId_ to generate request ID
			 */
			int nxtSequentialNumber_;
			boost::mutex nxtSequentialNumberMutex_;
    };
}

#endif  //_MESSAGE_CLIENT_LIGHT_H_
