/**
 */

#ifndef _MESSAGE_SERVER_LIGHT_H_
#define _MESSAGE_SERVER_LIGHT_H_

#include <net/message-framework/MessageControllerLight.h>

#include <string>
#include <vector>


namespace messageframework
{
    class MessageServerLight
    {
        public:

            /**
             * @brief Sets the server name and reference to the controller
             *
             * @details
             * The name of the server is used to identify the server among other servers in the same network.
             */
            MessageServerLight( const std::string & serverName, MessageControllerLight & controller )
                : serverName_(serverName), controller_(controller)
            {
                thisNode_.nodeName_ = serverName_;
            }


            /**
             * @brief   This function register a service of the current MessageServerFull. It will 
             *          submit the registration information of the service to the MessageController.
             * @param serviceName       The service name
             * @param parameterList     The list of parameter types of the service
             * @param permissionFlag    this flags tell how to serve the service (through Message Controller or 
             *                          directly at MessageServerFull)
             * @return                  True, if the service has been successfully registered.
             */
            bool registerService( 
                    const std::string & serviceName,
                    const std::vector<messageframework::ServiceParameterType> & parameterTypeList, 
                    const messageframework::PermissionFlag & permissionFlag,
                    const ServiceResultFlag serviceResultFlag = SERVICE_WITH_RESULT 
                    );

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
             *  requestList - the list of waiting service requests
             * @return
             *  true - the list is retrieved successfully
             */
            bool getServiceRequestList(std::vector<ServiceRequestInfoPtr>& requestList);

            /**
             * @brief This function answers a service request by putting the ServiceResult to the
             * requester of the service.
             * @param
             *  serviceRequest - information about the service request (service name, requester, parameter list)
             * @param
             *  result - the result of the service
             * @return
             *  true - the service result has been successfully put to the requester
             */
          //  bool putResultOfService(const ServiceRequestInfo& serviceRequestInfo, const ServiceResult& result);
            bool putResultOfService(const ServiceResultPtr& result);


        private:

            std::string serverName_;

            MessageControllerLight & controller_;

            MessageFrameworkNode thisNode_;

            //TODO: keep all service lists
    };
}


#endif  //_MESSAGE_SERVER_LIGHT_H_
