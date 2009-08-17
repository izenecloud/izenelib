/// 
/// @file ServiceRegistrationMessage.cpp
/// @date 14/8/2008
/// @author TuanQuang Nguyen
//


#include <net/message-framework/ServiceRegistrationMessage.h>

namespace messageframework
{
	/**
	 * @brief Default constructor is must be defined. boost::serialization will use
	 * this constructor
	 */
	ServiceRegistrationMessage::ServiceRegistrationMessage()
	{
		//registered_ = false;
		//serviceInfo_.clear();
		//requester_.clear();
	}

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
	/*ServiceRegistrationMessage::ServiceRegistrationMessage(const std::string& serviceName,
				const std::vector<ServiceParameterType> parameterList,
				PermissionFlag permissionFlag,
                ServiceResultFlag serviceResultFlag )
	{
		registered_ = false;
		//serviceInfo_.setPermissionFlag(permissionFlag);
		serviceInfo_.setServiceName(serviceName);
		//serviceInfo_.setParameterList(parameterList);
        //serviceInfo_.setServiceResultFlag( serviceResultFlag );
		requester_.clear();
	}*/

	void ServiceRegistrationMessage::setRequester(const std::string& requester)
	{
		requester_ = requester;
	}


	/**
	 * @brief Set server information
	 * @param
	 * server - IP address and port of the server
	 */
	void ServiceRegistrationMessage::setServer(const MessageFrameworkNode& server)
	{
		serviceInfo_.setServer(server.nodeIP_, server.nodePort_);
	}

	/**
	 * @brief get the reference to  Service Info of the message
	 * @return
	 * reference to the ServiceInfo
	 */
	//const ServiceInfo& ServiceRegistrationMessage::getServiceInfo() const
	//{return serviceInfo_;}

}// end of namespace messageframework

