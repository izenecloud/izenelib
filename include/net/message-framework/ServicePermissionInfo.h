///
///  @file : ServicePermissionInfo.h
///  @date : 8/5/2008
///  @author : TuanQuang Nguyen
///


#if !defined(_SERVICEPERMISSIONINFO_H)
#define _SERVICEPERMISSIONINFO_H

#include <net/message-framework/MessageFrameworkNode.h>
//#include <net/message-framework/ServiceParameterType.h>

#include <map>
#include <string>
#include <iostream>


namespace messageframework
{
	/**
	 * @brief This class defines the permission information. The permission information
	 * is created at Message Controller when there is a service request from a client.
	 */
	class ServicePermissionInfo {

	public:
		/**
		 * @brief The constructor, initialize variables with default values
		 */
		ServicePermissionInfo();

		//ServicePermissionInfo(const ServicePermissionInfo& servicePermissionInfo);

		/**
		 * @brief The destructor
		 */
		~ServicePermissionInfo();

		void clear();

		void setServiceName(const std::string& serviceName);

		const std::string& getServiceName() const;

	/*void setServiceNameId(const unsigned int& serviceNameId){
			serviceNameId_ = serviceNameId;
			}

	   const unsigned int getServiceNameId() const{
			return serviceNameId_;
			}*/

		/**
 		 * @brief set the server of the service
 	 	 * @param
 	 	 * ipAddress - IPAddress of the server
 	 	 * @param
 	 	 * port - port of the server
 	 	 */
		//void setServer(const std::string& ipAddress, unsigned int port);

		// ADDED @by MyungHyun (Kent) -2008-11-27
		//void setServer( const MessageFrameworkNode & server );
		
		void setServer(const std::string& agentInfo,  const MessageFrameworkNode server );
		
		void removeServer(const std::string& agentInfo ){
				agentInfoMap_.erase(agentInfo);
			}

		MessageFrameworkNode getServer(const std::string& agentInfo){
			return agentInfoMap_[agentInfo];
		}
		
		const  std::map<std::string, MessageFrameworkNode>& getServerMap() const {
				return agentInfoMap_;
			}
		
		bool   getServerMap(std::map<std::string, MessageFrameworkNode>& servers) {
			servers = agentInfoMap_;
			return true;
		}

		//void setPermissionFlag(PermissionFlag permissionFlag);

		/**
 	 	 * @brief Set the permission flag of the service
 	 	 * @param
 	 	 * permissionFlag - the permission flag
 	 	 */
		//const PermissionFlag& getPermissionFlag()const ;

		/**
		 * @brief Sets the return flag of this service
		 */
		//void setServiceResultFlag( const ServiceResultFlag flag )
		//{
		//  serviceResultFlag_ = flag;
		//}

		/**
		 * @brief Returns the return flag of this service
		 */
		//ServiceResultFlag getServiceResultFlag() const
		//{
		//  return serviceResultFlag_;
		//}

		template <typename Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			//ar & serviceName_ & serviceNameId_ &permissionFlag_ & server_ & serviceResultFlag_;
			//ar & serviceName_ & serviceNameId_ &permissionFlag_ & agentInfoMap_ & serviceResultFlag_;
			ar & serviceName_  & agentInfoMap_ ;
		}
		
		void display(std::ostream& os=std::cout) const
		{
			os<<serviceName_<<std::endl;
			os<<"agentInfoMap:"<<std::endl;
			std::map<std::string, MessageFrameworkNode>::const_iterator it;
			for(it=agentInfoMap_.begin(); it != agentInfoMap_.end(); it++)
			{
				os<<it->first<<" -> ";
				it->second.display(os);
			}
			
			
		}

	private:
		/**
		 * @brief the service name
		 */
		std::string serviceName_;

		//unsigned int serviceNameId_;

		/**
		 * @brief How the service is served
		 */
		//PermissionFlag permissionFlag_;

		/**
		 * @brief the server of the service. Depending on the permissionFlag_, the
		 * server can be a Message Controller or a Message Server.
		 */
		//MessageFrameworkNode server_;
		std::map<std::string, MessageFrameworkNode>  agentInfoMap_;
		

		/**
		 * @brief   Shows if this service returns a result
		 */
		//ServiceResultFlag serviceResultFlag_;
	};

	 //typedef boost::shared_ptr<ServicePermissionInfo> ServicePermissionInfoPtr;
}// end of messageframework

#endif  //_SERVICEPERMISSIONINFO_H


