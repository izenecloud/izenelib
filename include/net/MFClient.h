**
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

#include <message_framework.h>
#include <MFClientBase.h>

namespace messageframework {

	class MFClient {

	public:
		MFClient(){}
		
		MFClient(const MessageClientPtr& client):client_(client){}
		
		void setMessageClient(const MessageClientPtr& client){
			client_ = client;			
		}
		
		void displayAgentInfo(std::ostream& os = cout);
		
	protected:
		/**
		 *   requst with no result with agentInfo(collectionname)
		 */
		bool requestService(const std::vector<std::string>& agentInfos, const std::string& serviceName,
				const ServiceRequestInfoPtr& request,
		);
		/**
		 *   request with result with agentInfo(collectionname)
		 */
		bool requestService(const std::vector<std::string>& agentInfos, const std::string& serviceName,
				const ServiceRequestInfoPtr& serviceRequestInfo,
				serviceResultPtr& result
		);

		/**
		 *   requst with no result to all agents
		 */
		bool requestService( const std::string& serviceName,
				const ServiceRequestInfoPtr& request,
		);
		/**
		 *   request with result to all agents
		 */
		bool requestService( const std::string& serviceName,
				const ServiceRequestInfoPtr& serviceRequestInfo,
				serviceResultPtr& result
		);
		
		
	private:
		bool requestOne_(const ServiceRequestInfoPtr& request, serviceResultPtr& result);		
		
	private:	
		MessageClientPtr client_;

	};

}

#endif
