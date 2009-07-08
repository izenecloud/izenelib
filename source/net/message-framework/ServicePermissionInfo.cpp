///
///  @file : ServicePermissionInfo.cpp
///  @date : 8/5/2008
///  @author : TuanQuang Nguyen
///


#include <net/message-framework/MessageFrameworkNode.h>
#include <net/message-framework/ServiceParameterType.h>
#include <net/message-framework/ServicePermissionInfo.h>

#include <string>

namespace messageframework
{
		ServicePermissionInfo::ServicePermissionInfo():
			serviceName_(""),
			permissionFlag_(SERVE_THROUGH_CONTROLLER)
		{
			server_.nodeIP_ = "";
			server_.nodePort_ = 0;
			serviceResultFlag_ = SERVICE_WITH_RESULT;
		}

		ServicePermissionInfo::ServicePermissionInfo(
			const ServicePermissionInfo& servicePermissionInfo)
		{
			serviceName_ = servicePermissionInfo.getServiceName();
			permissionFlag_ = servicePermissionInfo.getPermissionFlag();
			server_ = servicePermissionInfo.getServer();
			serviceResultFlag_ = servicePermissionInfo.getServiceResultFlag();
		}

		/**
		 * @brief The destructor
		 */
		ServicePermissionInfo::~ServicePermissionInfo(){}

		void ServicePermissionInfo::clear()
		{
			serviceName_.clear();
			permissionFlag_ = UNKNOWN_PERMISSION_FLAG;
			server_.clear();
			serviceResultFlag_ = SERVICE_WITH_RESULT;
		}

		const std::string& ServicePermissionInfo::getServiceName() const
		{
			return serviceName_;
		}

		void ServicePermissionInfo::setServiceName(const std::string& serviceName){ serviceName_ = serviceName;}

		// ADDED @by MyungHyun (Kent) -2008-11-27
		void ServicePermissionInfo::setServer( const MessageFrameworkNode & server )
		{
		  server_ = server;
		}

		const MessageFrameworkNode& ServicePermissionInfo::getServer()const
		{
			return server_;
		}

		const PermissionFlag& ServicePermissionInfo::getPermissionFlag()const
		{
			return permissionFlag_;
		}

		/**
 		 * @brief set the server of the service
 	 	 * @param
 	 	 * ipAddress - IPAddress of the server
 	 	 * @param
 	 	 * port - port of the server
 	 	 */
		void ServicePermissionInfo::setServer(const std::string& ipAddress, unsigned int port)
		{
			server_.nodeIP_ = ipAddress;
			server_.nodePort_ = port;
		}

		/**
 	 	 * @brief Set the permission flag of the service
 	 	 * @param
 	 	 * permissionFlag - the permission flag
 	 	 */
		void ServicePermissionInfo::setPermissionFlag(PermissionFlag permissionFlag)
		{
			permissionFlag_ = permissionFlag;
		}

}// end of messageframework

