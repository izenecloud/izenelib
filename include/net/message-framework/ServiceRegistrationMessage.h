/// 
/// @file ServiceRegistrationMessage.h
/// @date 14/8/2008
/// @author TuanQuang Nguyen
//


#ifndef SERVICE_REGISTRATION_MESSAGE_H_
#define SERVICE_REGISTRATION_MESSAGE_H_

#include <net/message-framework/ServiceInfo.h>

#include <vector>
#include <string>

namespace messageframework {

/**
 * @brief This class defines message between MessageServer and MessageController
 * when MessageServer registers its services to MessageController.
 */
class ServiceRegistrationMessage {
public:
	/**
	 * @brief Default constructor is must be defined. boost::serialization will use
	 * this constructor
	 */
	ServiceRegistrationMessage();

	/**
	 * @brief Construct a ServiceRegistrationMessage given a service name,
	 * list of parameters, and permissionFlag. The permissionFlag defines
	 * how the service is served (at server or through controller)
	 * @param
	 * serviceName - service name
	 * @param
	 * parameterList - the parameter list
	 * @param
	 * permissionFlag - the permission flag
	 */
	/*ServiceRegistrationMessage(const std::string& serviceName,
			const std::vector<ServiceParameterType> parameterList,
			PermissionFlag permissionFlag,
			ServiceResultFlag serviceResultFlag = SERVICE_WITH_RESULT);*/
	
	void setServiceInfo(const ServiceInfo& serviceInfo){
		serviceInfo_ = serviceInfo;
	}
	


	void setRequester(const std::string& requester);

	/**
	 * @brief Set server information
	 * @param
	 * server - IP address and port of the server
	 */
	void setServer(const MessageFrameworkNode& server);
	
	const MessageFrameworkNode& getServer() const {
		return serviceInfo_.getServer();
	}

	/**
	 * @brief get the reference to  Service Info of the message
	 * @return
	 * reference to the ServiceInfo
	 */
	
	const ServiceInfo& getServiceInfo() const{
		return serviceInfo_;
	}

	/**
	 * @brief set AgentInfo 
	 * 
	 */
	void setAgentInfo(const std::string& agentInfo){agentInfo_ = agentInfo;}
	
	const std::string& getAgentInfo() const {return agentInfo_;}
	
	
	const std::string& getServiceName() const {
		return serviceInfo_.getServiceName();
	}
	

	/**
	 * @brief This function is used by boost:serialization
	 */
	template <typename Archive> void serialize(Archive & ar,
			const unsigned int version) {
		ar & serviceInfo_;
		//ar & registered_;
		ar & requester_;
		ar & agentInfo_;
	}
	void display(std::ostream &os = std::cout) const {
		os<<"ServiceRegistrationMessage: "<<std::endl;
		os<<"agentInfo: "<<agentInfo_<<std::endl;
		serviceInfo_.display(os);
	}
private:
	ServiceInfo serviceInfo_;
	//bool registered_;
	std::string requester_;
	std::string agentInfo_;

};

// typedef boost::shared_ptr<ServiceRegistrationMessage> ServiceRegistrationMessagePtr;

}// end of namespace messageframework

#endif
