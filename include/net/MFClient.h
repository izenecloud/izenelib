/**
* @brief Defines the MFClient class
* @author Peisheng Wang
*
* @brief provide interface between application level and message_framework level.
*
* @date 08-17-2009
* @details
*
*/

#ifndef _MFCLIENT_H_
#define _MFCLIENT_H_

#include <net/message_framework.h>
#include <net/MFClientBase.h>

namespace messageframework {

	class MFClient {

	public:
		MFClient() {}

		MFClient(const MessageClientPtr& client):client_(client) {}

		void setMessageClient(const MessageClientPtr& client) {
			client_ = client;
		}

		bool getAgentInfo(const string& serviceName, std::vector<std::string>& agentInfos);

		void displayAgentInfo(const std::string& serviceName, std::ostream& os=std::cout) const;

	protected:
		bool getHostsOfService(const std::string& serviceName, std::map<std::string, MessageFrameworkNode>& servers){
			return client_->getHostsOfService(serviceName, servers);
		}
		/**
		 *   requst with no result with agentInfo(collectionname)
		 */
		bool requestService(const std::vector<std::string>& agentInfos, const std::string& serviceName,
				const ServiceRequestInfoPtr& request
		);
		/**
		 *   request with result with agentInfo(collectionname)
		 */
		bool requestService(const std::vector<std::string>& agentInfos, const std::string& serviceName,
				const ServiceRequestInfoPtr& serviceRequestInfo,
				std::vector<ServiceResultPtr>& results
		);

		/**
		 *   requst with no result to all agents
		 */
		bool requestService( const std::string& serviceName,
				const ServiceRequestInfoPtr& request
		);
		/**
		 *   request with result to all agents
		 */
		bool requestService( const std::string& serviceName,
				const ServiceRequestInfoPtr& serviceRequestInfo,
				std::vector<ServiceResultPtr>& results
		);		

		/**
		 *   requst with no result to one agent
		 */
		bool requestService(const std::string&agentInfo, const std::string& serviceName,
				const ServiceRequestInfoPtr& request
		);
		/**
		 *   request with result to to one agent
		 */
		bool requestService(const std::string&agentInfo, const std::string& serviceName,
				const ServiceRequestInfoPtr& serviceRequestInfo,
				ServiceResultPtr& result
		);
		
		MessageClientPtr client_;

	private:
		bool requestOne_(const MessageFrameworkNode& server, const ServiceRequestInfoPtr& request);
		bool requestOne_(const MessageFrameworkNode& server, const ServiceRequestInfoPtr& request, ServiceResultPtr& result);

	};

}

#endif
