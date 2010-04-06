#include <net/message-framework/MessageServerLight.h>

namespace messageframework {

bool MessageServerLight::registerService(const ServiceInfo & serviceInfo) {
	ServiceRegistrationMessage message;
	message.setAgentInfo( agentInfo_ );
	message.setServiceInfo( serviceInfo );
	message.setServer(thisNode_);

	return controller_.addServiceRegistrationRequest(message);
}

bool MessageServerLight::getServiceRequestList(
		std::vector<ServiceRequestInfoPtr> & requestList) {
	return controller_.getRequestListByServer(thisNode_, requestList);
}

bool MessageServerLight::putResultOfService(const ServiceResultPtr & result) {
	return controller_.addResultOfService(result);
}

}
