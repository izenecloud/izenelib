///
///  @file : PermissionRequester.h
///  @date : 20/11/2008
///  @author : TuanQuang Nguyen
///  @brief This file define PermissionRequester abstract class
///



#ifndef _PERMISSION_REQUESTER_H_
#define _PERMISSION_REQUESTER_H_

#include <net/message-framework/ServicePermissionInfo.h>

#include <string>


namespace messageframework
{
	/**
     * @brief This class defines interfaces of object that requests permission
     * of service
     */
	class PermissionRequester
	{
	public:
		PermissionRequester(){}
		virtual ~PermissionRequester(){}

		/**
 		 * @brief This function sends a request to get Permission of a service
 		 * @param serviceName the service name
 		 */
		virtual void sendPermissionOfServiceRequest(const std::string& serviceName) = 0;

		/**
 		 * @brief When retrieving permission of service from controller, this function
 		 * is called in order to reply to the request to get permission
 		 * @param servicePermissionInfo the new permission of service
 		 */
		virtual void receivePermissionOfServiceResult(const ServicePermissionInfo& servicePermissionInfo) = 0;
	};
}// end of namespace messageframework

#endif

