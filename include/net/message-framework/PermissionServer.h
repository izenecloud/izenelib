///
///  @file : PermissionServer.h
///  @date : 20/11/2008
///  @author : TuanQuang Nguyen
///  @brief This file define PermissionServer abstract class
///


#ifndef _PERMISSION_SERVER_H_
#define _PERMISSION_SERVER_H_

#include <net/message-framework/ServicePermissionInfo.h>
#include <net/message-framework/MessageFrameworkNode.h>

#include <string>


namespace messageframework
{
	/**
     * @brief This class defines interfaces of object that serves requests of permission
     * of service
     */
	class PermissionServer
	{
	public:
		PermissionServer(){}
		virtual ~PermissionServer(){}

		/**
 		 * @brief When a new request of permision comes, this function
 		 * is called in order to notify the server 
 	 	 * @param requester node that requests a permission
 		 * @param serviceName the service name to get permission
 		 */
		virtual void receivePermissionOfServiceRequest(const MessageFrameworkNode& requester, 
								const std::string& serviceName ) = 0;

		/**
 	 	 * @brief This function replies to a request from PermissionRequester
 	 	 * @param requester node that requests for a permission
 	 	 * @param permission the permission
 	 	 */
		virtual void sendPermissionOfServiceResult(const MessageFrameworkNode& requester, 
							const ServicePermissionInfo& permission) = 0;
	};
}// end of namespace messageframework

#endif // #ifndef _PERMISSION_SERVER_H_

