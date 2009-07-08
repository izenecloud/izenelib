///
///  @file : ServicePermissionRequest.h
///  @date : 8/5/2008
///  @author : TuanQuang Nguyen
///


#if !defined(_SERVICEPERMISSIONREQUEST_H)
#define _SERVICEPERMISSIONREQUEST_H

#include <net/message-framework/MessageFrameworkNode.h>

#include <string>


namespace messageframework
{
	/**
	 * @brief This class define a permission request. It consists of
	 * information of the requester and the service name
	 */
	class ServicePermissionRequest {
	public:
		/**
		 * @brief constructor, initialize variables with default values.
		 */
		ServicePermissionRequest()serviceName_("")
		{
			requester_.nodeIP_ = 0; requester_.nodePort_ = 0;
		}

		ServicePermissionRequest(const ServicePermissionRequest& servicePermissionRequest)
		{
			requester_ = servicePermissionRequest.requester_;
			serviceName_ = servicePermissionRequest.serviceName_;
		}

	public:
		/**
		 * @brief the IP and port of the requester
		 */
		MessageFrameworkNode requester_;

		/**
		 * @brief the service name
		 */
		std::string serviceName_;
	};
}// end of messageframework
#endif  //_SERVICEPERMISSIONREQUEST_H
