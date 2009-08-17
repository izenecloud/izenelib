///
///  @file  ServiceRegistrationServer.h
///  @date  21/11/2008
///  @author  TuanQuang Nguyen
///  @brief This file defines ServiceRegistrationServer that 
///  is serves service registration requests.

#ifndef _SERVICE_REGISTRATION_SERVER_H_
#define _SERVICE_REGISTRATION_SERVER_H_

#include <net/message-framework/MessageFrameworkNode.h>
#include <net/message-framework/ServiceRegistrationMessage.h>

namespace messageframework
{
/**
 * @brief This class is used to serves ServiceRegistration requests
 */
class ServiceRegistrationServer
{
public:
	ServiceRegistrationServer(){}
	virtual ~ServiceRegistrationServer(){}

	/**
 	 * @brief This function puts the registration requests to the queue
 	 * of the server
 	 * @param localEndPoint the end point that connects to the ServiceRegistrationRequester
 	 * @param serviceInfo the service information to register
 	 */
	virtual void receiveServiceRegistrationRequest(const MessageFrameworkNode& localEndPoint, const ServiceRegistrationMessage& registMessage)= 0;

	/**
 	 * @brief This function replies to a registration request from ServiceRegistrationRequester. It
 	 * will sends registration result to the ServiceRegistrationRequester.
 	 * @param localEndPoint the end point that connects to the ServiceRegistrationClient.
 	 * @param serviceInfo the service information to register
 	 * @param success the registration result
 	 */
	virtual void sendServiceRegistrationResult(const MessageFrameworkNode& localEndPoint, const ServiceInfo& serviceInfo, 
			bool success) = 0;

};
}// end of namespace messageframework
#endif

