/**
 * @author  MyungHyun Lee (Kent), Rewritten by Wei Cao 2009-3-29
 */

#include <net/message-framework/MessageControllerLight.h>
#include <net/message-framework/MessageClientLight.h>
#include <iostream>
#include <util/izene_log.h>

using namespace std;
using namespace boost;

namespace messageframework {

void MessageControllerLight::processServiceRegistrationRequest() {
	boost::mutex::scoped_lock lock(processDummyMutex_);
	processDummyCond_.wait(lock);
}

void MessageControllerLight::processServicePermissionRequest() {
	boost::mutex::scoped_lock lock(processDummyMutex_);
	processDummyCond_.wait(lock);
}

void MessageControllerLight::processServiceResultFromServer() {
	boost::mutex::scoped_lock lock(processDummyMutex_);
	processDummyCond_.wait(lock);
}

void MessageControllerLight::processServiceRequestFromClient() {
	boost::mutex::scoped_lock lock(processDummyMutex_);
	processDummyCond_.wait(lock);
}

int MessageControllerLight::applyForClientId() {
	{
		boost::mutex::scoped_lock lock(nxtClientIdMutex_);
		if (nxtClientId_ == MF_LIGHT_MAX_CLIENT_ID) {
			std::cout << "Error: Max Client ID reached" << std::endl;
			return 0;
		}

		DLOG(INFO)<<"applyForClientId"<<nxtClientId_<<endl;
		serviceIdResultMaps_[nxtClientId_].reset(new std::map<unsigned int, ServiceResultPtr>);
		serviceIdResultMutexs_[nxtClientId_].reset(new boost::mutex() );
		serviceIdResultConds_[nxtClientId_].reset(new boost::condition_variable() );

		int clientId = nxtClientId_;
		nxtClientId_ ++;
		return clientId;
	}
}

bool MessageControllerLight::addServiceRegistrationRequest(
		const ServiceRegistrationMessage & message) {

	const MessageFrameworkNode & server = message.getServer();
	boost::mutex::scoped_lock lock(availableServiceListMutex_);
	std::string serviceName = message.getServiceName();

	boost::unordered_map<std::string, ServicePermissionInfo>::iterator it;
	it = availableServiceList_.find(serviceName);
	
	if (it != availableServiceList_.end() ) {
		it->second.setServer(message.getAgentInfo(), server);
	} else {
		ServicePermissionInfo permissionInfo;
		permissionInfo.setServiceName(serviceName);
		permissionInfo.setServer(message.getAgentInfo(), server);
		availableServiceList_.insert(make_pair(serviceName, permissionInfo) );
	}
	

	DLOG(INFO)<<"MessageControllerLight::addServiceRegistrationRequest()"<<endl;
	//availableServiceList_[serviceName].display();
	

	boost::mutex::scoped_lock lock_map(serverServiceRequestQueueMapMutex_);
	boost::shared_ptr<vector<shared_ptr<ServiceRequestInfo> > >
			serverServiceRequestQueue(new vector<shared_ptr<ServiceRequestInfo> >());
	boost::shared_ptr<boost::mutex>
			serverServiceRequestQueueMutex(new boost::mutex());
	boost::shared_ptr<boost::condition_variable>
			serverServiceRequestQueueCond(new boost::condition_variable());
	serverServiceRequestQueueMap_[server.hash()] = serverServiceRequestQueue;
	serverServiceRequestQueueMutexMap_[server.hash()] = serverServiceRequestQueueMutex;
	serverServiceRequestQueueCondMap_[server.hash()] = serverServiceRequestQueueCond;

	return true;

}

bool MessageControllerLight::getServicePermission(
		const std::string & serviceName,
		ServicePermissionInfo & servicePermissionInfo) {

	boost::mutex::scoped_lock lock(availableServiceListMutex_);
	boost::unordered_map<std::string, ServicePermissionInfo>::iterator it;

	if ( (it = availableServiceList_.find(serviceName))
			!= availableServiceList_.end()) {
		servicePermissionInfo = availableServiceList_[serviceName];
		return true;
	} else {
		servicePermissionInfo.clear();
		servicePermissionInfo.setServiceName(serviceName);
		return false;
	}
}

bool MessageControllerLight::addServiceRequest(
		const MessageFrameworkNode & server,
		const ServiceRequestInfoPtr & requestInfo) {
	{
		boost::mutex::scoped_lock lock(availableServiceListMutex_);
		if (availableServiceList_.find(requestInfo->getServiceName() )
				== availableServiceList_.end() ) {
			cout << "CONTROLLER:\t  service does *NOT* exists: "
					<< requestInfo->getServiceName() << endl;
			return false;
		}
	}

	boost::shared_ptr<std::vector<ServiceRequestInfoPtr> >
			serverServiceRequestQueue;
	boost::shared_ptr<boost::mutex> serverServiceRequestQueueMutex;
	boost::shared_ptr<boost::condition_variable> serverServiceRequestQueueCond;
	{
		boost::mutex::scoped_lock lock_map(serverServiceRequestQueueMapMutex_);
		serverServiceRequestQueue = serverServiceRequestQueueMap_[server.hash()];
		serverServiceRequestQueueMutex
				= serverServiceRequestQueueMutexMap_[server.hash()];
		serverServiceRequestQueueCond
				= serverServiceRequestQueueCondMap_[server.hash()];
	}

	{
		boost::mutex::scoped_lock lock_queue(*serverServiceRequestQueueMutex);
		serverServiceRequestQueue->push_back(requestInfo);
	}
	serverServiceRequestQueueCond->notify_all();

	return true;
}

bool MessageControllerLight::addServiceRequests(
		const MessageFrameworkNode & server,
		const std::vector<ServiceRequestInfoPtr> & requestInfos) {

	// sanity checks
	if (requestInfos.size() == 0) {
		std::cout << "Warning: requestInfos.size() == 0 in addServiceRequests"
				<< std::endl;
		return true;
	}

	std::string serviceName = requestInfos[0]->getServiceName();
	for (unsigned int i=1; i<requestInfos.size(); i++) {
		if (requestInfos[i]->getServiceName() != serviceName) {
			std::cout
					<< "Error: all requetInfos should be with the same service in addServiceRequests"
					<< std::endl;
			return true;
		}
	}

	{
		boost::mutex::scoped_lock lock(availableServiceListMutex_);
		if (availableServiceList_.find(serviceName)
				== availableServiceList_.end() ) {
			cout << "Error: service does *NOT* exists: " << serviceName << endl;
			return false;
		}
	}

	// insert requests into queue
	boost::shared_ptr<std::vector<ServiceRequestInfoPtr> >
			serverServiceRequestQueue;
	boost::shared_ptr<boost::mutex> serverServiceRequestQueueMutex;
	boost::shared_ptr<boost::condition_variable> serverServiceRequestQueueCond;
	{
		boost::mutex::scoped_lock lock_map(serverServiceRequestQueueMapMutex_);
		serverServiceRequestQueue = serverServiceRequestQueueMap_[server.hash()];
		serverServiceRequestQueueMutex
				= serverServiceRequestQueueMutexMap_[server.hash()];
		serverServiceRequestQueueCond
				= serverServiceRequestQueueCondMap_[server.hash()];
	}

	{
		boost::mutex::scoped_lock lock_queue(*serverServiceRequestQueueMutex);
		serverServiceRequestQueue->reserve(serverServiceRequestQueue->size()
				+ requestInfos.size() );
		for (unsigned int i=0; i<requestInfos.size(); i++)
			serverServiceRequestQueue->push_back(requestInfos[i]);
	}
	serverServiceRequestQueueCond->notify_all();

	return true;
}

bool MessageControllerLight::getRequestListByServer(
		const MessageFrameworkNode & server,
		std::vector<ServiceRequestInfoPtr> & requestList) {
	requestList.clear();

	boost::shared_ptr<std::vector<ServiceRequestInfoPtr> >
			serverServiceRequestQueue;
	boost::shared_ptr<boost::mutex> serverServiceRequestQueueMutex;
	boost::shared_ptr<boost::condition_variable> serverServiceRequestQueueCond;
	{
		boost::mutex::scoped_lock lock_map(serverServiceRequestQueueMapMutex_);
		serverServiceRequestQueue = serverServiceRequestQueueMap_[server.hash()];
		serverServiceRequestQueueMutex
				= serverServiceRequestQueueMutexMap_[server.hash()];
		serverServiceRequestQueueCond
				= serverServiceRequestQueueCondMap_[server.hash()];
	}

	{
		boost::mutex::scoped_lock lock_queue(*serverServiceRequestQueueMutex);

		while (serverServiceRequestQueue->size() == 0)
			serverServiceRequestQueueCond->wait(lock_queue);
		requestList.reserve(serverServiceRequestQueue->size());
		for (unsigned int i=0; i<serverServiceRequestQueue->size(); i++) {
			requestList.push_back( ((*serverServiceRequestQueue)[i]));
		}
		serverServiceRequestQueue->clear();

		if (requestList.size() > 0)
			return true;
		return false;
	}
}

bool MessageControllerLight::addResultOfService(
		const ServiceResultPtr & serviceResult) {

	unsigned int requestId = serviceResult->getRequestId();
	unsigned int clientId = (requestId >> MF_LIGHT_REQUEST_ID_CLIENT_ID_SHIFT );

	if (clientId == 0 || clientId > MF_LIGHT_MAX_CLIENT_ID) {
		std::cout << "Bad clientId " << clientId << std::endl;
		return false;
	}

	boost::shared_ptr<std::map<unsigned int, ServiceResultPtr> >
			serviceIdResultMap = serviceIdResultMaps_[clientId];
	boost::shared_ptr<boost::mutex> serviceIdResultMutex =
			serviceIdResultMutexs_[clientId];
	boost::shared_ptr<boost::condition_variable> serviceIdResultCond =
			serviceIdResultConds_[clientId];

	{
		boost::mutex::scoped_lock lock( *serviceIdResultMutex);
		(*serviceIdResultMap)[requestId] = serviceResult;
	}
	serviceIdResultCond->notify_all();

	return true;
}

bool MessageControllerLight::getResultByRequestId(const unsigned int requestId,
		ServiceResultPtr & result) {
	unsigned int clientId = (requestId >> MF_LIGHT_REQUEST_ID_CLIENT_ID_SHIFT );
	if (clientId == 0 || clientId > MF_LIGHT_MAX_CLIENT_ID) {
		std::cout << "Bad clientId " << clientId << std::endl;
		return false;
	}

	boost::shared_ptr<std::map<unsigned int, ServiceResultPtr> >
			serviceIdResultMap = serviceIdResultMaps_[clientId];
	boost::shared_ptr<boost::mutex> serviceIdResultMutex =
			serviceIdResultMutexs_[clientId];
	boost::shared_ptr<boost::condition_variable> serviceIdResultCond =
			serviceIdResultConds_[clientId];

	{
		boost::mutex::scoped_lock lock( *serviceIdResultMutex);
		while (serviceIdResultMap->find(requestId) == serviceIdResultMap->end() )
			serviceIdResultCond->wait(lock);

		result = (*serviceIdResultMap)[requestId];
		serviceIdResultMap->erase(requestId);
	}

	return true;
}

void MessageControllerLight::printAvailServiceList() {
	using namespace std;
	boost::unordered_map<std::string, ServicePermissionInfo>::iterator it;
	for (it = availableServiceList_.begin(); it != availableServiceList_.end(); it++) {
		cout << "service name: " << it->first << endl;
	}
}

}
