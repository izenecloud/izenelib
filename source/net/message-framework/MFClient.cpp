#include <net/MFClient.h>

bool MFClient::getAgentInfo(const std::string& serviceName,
		std::vector<std::string>& agentInfos) {
	agentInfos.clear();
	std::map<std::string, MessageFrameworkNode> hosts;

	if ( !client_->getPermissionOfService(serviceName, hosts) )
		return false;

	std::map<std::string, MessageFrameworkNode>::const_iterator it =
			hosts.begin();
	for (; it != hosts.end(); it++) {
		agentInfos.push_back(it->first);
	}
	return true;

}

bool MFClient::requestService(const std::string& serviceName,
		const ServiceRequestInfoPtr& request) {
	std::map<std::string, MessageFrameworkNode> hosts;

	if ( !client_->getPermissionOfService(serviceName, hosts) )
		return false;

	std::map<std::string, MessageFrameworkNode>::const_iterator it =
			hosts.begin();
	for (; it != hosts.end(); it++) {
		if ( !requestOne_(it->second, request) )
			DLOG(ERROR) << "putServiceRequest failed for "<<it->first<<endl;
	}
	return true;

}

bool MFClient::requestService(const std::string& serviceName,
		const ServiceRequestInfoPtr& request,
		std::vector<ServiceResultPtr>& results) {
	std::map<std::string, MessageFrameworkNode> hosts;

	if ( !client_->getPermissionOfService(serviceName, hosts) )
		return false;

	std::map<std::string, MessageFrameworkNode>::const_iterator it =
			hosts.begin();

	ServiceResultPtr temp;
	for (; it != hosts.end(); it++) {
		if (requestOne_(it->second, request, temp) ) {
			temp->setAgentInfo(it->first);
			results.push_back(temp);
		}
	}
	return true;

}

bool MFClient::requestService(const std::vector<std::string>& agentInfos,
		const std::string& serviceName, const ServiceRequestInfoPtr& request) {
	std::map<std::string, MessageFrameworkNode> hosts;

	if ( !client_->getPermissionOfService(serviceName, hosts) )
		return false;

	std::map<std::string, MessageFrameworkNode>::iterator mit;

	for (unsigned int i=0; i< agentInfos.size(); i++) {
		mit = hosts.find(agentInfos[i]);
		if (mit == hosts.end() ) {
			DLOG(ERROR) << "Server not found for "<<agentInfos[i]<<endl;
			continue;
		}
		if ( !requestOne_(mit->second, request) ) {
			DLOG(ERROR)<<"requst failed for agentInfo="<<agentInfos[i]<<endl;
		}

	}
	return true;
}

bool MFClient::requestService(const std::vector<std::string>& agentInfos,
		const std::string& serviceName, const ServiceRequestInfoPtr& request,
		std::vector<ServiceRequestInfoPtr>& results) {
	std::map<std::string, MessageFrameworkNode> hosts;

	if ( !client_->getPermissionOfService(serviceName, hosts) )
		return false;

	results.resize(agentInfos.size() );
	std::map<std::string, MessageFrameworkNode>::iterator mit;

	for (unsigned int i=0; i<agentInfos.size(); i++) {
		mit = hosts.find(agentInfos[i]);
		if (mit == hosts.end() ) {
			DLOG(ERROR) << "Server not found for "<<agentInfos[i]<<endl;
			continue;
		}
		if (requestOne_(mit->second, request, results[i]) ) {
			results[i]->setAgentInfo(agentInfos[i]);
		} else {
			DLOG(ERROR)<<"requst failed for agentInfo="<<agentInfos[i]<<endl;
		}
	}
	return true;
}

bool MFClient::requestService(const std::string& agentInfo,
		const std::string& serviceName, const ServiceRequestInfoPtr& request) {
	std::map<std::string, MessageFrameworkNode> hosts;

	if ( !client_->getPermissionOfService(serviceName, hosts) )
		return false;

	std::map<std::string, MessageFrameworkNode>::iterator mit;
	assert(client_ != 0);
	if (agentInfo == "#any") {
		mit = hosts.begin();
		for (; mit != hosts.end(); mit++) {
			if (requestOne_(mit->second, request) ) {
				return true;
				break;
			}
		}
		return false;
	}

	mit = hosts.find(agentInfo);
	if (mit == hosts.end() ) {
		DLOG(ERROR) << "Server not found for "<<agentInfo<<std::endl;
		return false;
	}
	if ( !requestOne_(mit->second, request) ) {
		DLOG(ERROR)<<"requst failed for agentInfo="<<agentInfo<<std::endl;
		return false;
	}

	return true;
}

bool MFClient::requestService(const std::string& agentInfo,
		const std::string& serviceName, const ServiceRequestInfoPtr& request,
		ServiceResultPtr& result) {
	std::map<std::string, MessageFrameworkNode> hosts;
	assert(client_ != 0);
	if ( !client_->getPermissionOfService(serviceName, hosts) )
		return false;

	std::map<std::string, MessageFrameworkNode>::iterator mit;
	if (agentInfo == "#any") {
		mit = hosts.begin();
		for (; mit != hosts.end(); mit++) {
			if (requestOne_(mit->second, request, result) ) {
				return true;
				break;
			}
		}
		return false;
	}

	mit = hosts.find(agentInfo);
	if (mit == hosts.end() ) {
		DLOG(ERROR) << "Server not found for "<<agentInfo<<std::endl;
		return false;
	}
	if (requestOne_(mit->second, request, result) ) {
		result->setAgentInfo(agentInfo);
	} else {
		DLOG(ERROR)<<"requst failed for agentInfo="<<agentInfo<<std::endl;
		return false;
	}

	return true;
}

bool MFClient::requestOne_(const MessageFrameworkNode& server,
		const ServiceRequestInfoPtr& request) {

	ServiceRequestInfoPtr newRequest(new ServiceRequestInfo);
	newRequest->setServiceName(request->getServiceName() );
	newRequest->setBuffer(request->bufferPtrVec);
	if ( !client_->putServiceRequest(server, newRequest, false) ) {
		DLOG(ERROR) << "failed to send service request";
		return false;
	}
	return true;

}

bool MFClient::requestOne_(const MessageFrameworkNode& server,
		const ServiceRequestInfoPtr& request, ServiceResultPtr& result) {

	ServiceRequestInfoPtr newRequest(new ServiceRequestInfo);
	newRequest->setServiceName(request->getServiceName() );
	newRequest->setBuffer(request->bufferPtrVec);

	if ( !client_->putServiceRequest(server, newRequest, true) ) {
		DLOG(ERROR) << "failed to send service request";
		return false;
	}
	if ( !client_->getResultOfService(newRequest, result) ) {
		DLOG(ERROR) << "failed to get service reply";
		return false;
	}

	return true;

}

void MFClient::displayAgentInfo(const std::string& serviceName, std::ostream& os) const {

	std::map<std::string, MessageFrameworkNode> hosts;
	if ( !client_->getPermissionOfService(serviceName, hosts) )
		return;
	std::map<std::string, MessageFrameworkNode>::const_iterator it =
			hosts.begin();
	for (; it != hosts.end(); it++) {
		os << it->first << ":" << std::endl;
		it->second.display(os);
	}

}
