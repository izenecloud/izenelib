///
///  @file  ServiceRegistrationRequester.h
///  @date  21/11/2008
///  @author  TuanQuang Nguyen
///  @brief This file defines ServiceRegistrationRequester that 
///  is used to register services


#ifndef _SERVICE_REGISTRATION_REQUESTER_H_
#define _SERVICE_REGISTRATION_REQUESTER_H_

//#include <net/message-framework/ServiceParameterType.h>
#include <net/message-framework/ServiceRegistrationMessage.h>

#include <string>
#include <vector>

namespace messageframework
{
/**
 * @brief this class is used to register a services
 */
class ServiceRegistrationRequester
{
	public:
		ServiceRegistrationRequester(){}
		virtual ~ServiceRegistrationRequester(){}

		/**
		 * @brief This function sends request to ServiceRegistrationServer 
		 * in order to register a service
		 * @param
		 *	serviceName - The service name
		 * @param
		 *  parameterList - The list of parameter types of the service
		 * @param
		 *	permissionFlag - this flags tell how to serve the service (through Message Controller or
		 *				directly at MessageServer)
		 */

		/*virtual void sendServiceRegistrationRequest(const std::string& serviceName,
				const std::vector<ServiceParameterType>& parameterList,
				const PermissionFlag& permissionFlag,
                const ServiceResultFlag serviceResultFlag ) = 0;*/
		virtual void sendServiceRegistrationRequest(ServiceRegistrationMessage& registMessage) = 0;
		/**
 		 * @brief This function sets the reply of the recent request of service registration
 		 * @param
 		 * serviceName - the service name to register
 		 * @param
 		 * success - true if the registration is successful
 		 */
		virtual void receiveServiceRegistrationReply(const std::string& serviceName, bool success) = 0;
};
}// end of  namespace messageframework
#endif
