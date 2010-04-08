///
///  @file : ServicePermissionInfo.cpp
///  @date : 8/5/2008
///  @author : TuanQuang Nguyen
///


#include <net/message-framework/MessageFrameworkNode.h>
//#include <net/message-framework/ServiceParameterType.h>
#include <net/message-framework/ServicePermissionInfo.h>

#include <string>
#include <vector>
using namespace std;

namespace messageframework {
ServicePermissionInfo::ServicePermissionInfo() :
	serviceName_("") {
}

/**
 * @brief The destructor
 */
ServicePermissionInfo::~ServicePermissionInfo() {
}

void ServicePermissionInfo::clear() {
}

const std::string& ServicePermissionInfo::getServiceName() const {
	return serviceName_;
}

void ServicePermissionInfo::setServiceName(const std::string& serviceName) {
	serviceName_ = serviceName;
}

void ServicePermissionInfo::setServer(const std::string& agentInfo,
		const MessageFrameworkNode server) {
	std::map<std::string, MessageFrameworkNode>::iterator it =
			agentInfoMap_.begin();
	//delete obsolete infos
	vector<string> toBeDeleted;
	for (; it != agentInfoMap_.end(); it++) {
		if (server == it->second)
			toBeDeleted.push_back(it->first);
	}
    for(vector<string>::iterator it=toBeDeleted.begin(); it != toBeDeleted.end(); it++)
    	agentInfoMap_.erase(*it);
	agentInfoMap_[agentInfo] = server;
}

}// end of messageframework

