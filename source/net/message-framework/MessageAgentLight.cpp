///#include <message-framework/Serializable.h>

#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <net/message-framework/MessageAgentLight.h>

using namespace std;
using namespace boost;

namespace messageframework {

//typedef MessageClientLight MessageClient;
/**
 * @brief for profiling - Wei Cao
 */
long permission_time = 0;
long send_time = 0;
long recv_time = 0;

/**
 * @brief Get the service permission info of a given service name
 * from cache if hit, or get it from controller
 */

//typedef boost::unordered_map<std::string, ServicePermissionInfo>
//		permission_cache_type;

///typedef std::pair<std::string, ServicePermissionInfo> permission_cache_element;

//static boost::mutex permissionCacheMutex_;

//static permission_cache_type permissionCache;


/*
 bool getServicePermissionInfo(const std::string& serviceName,
 ServicePermissionInfo& servicePermissionInfo, MessageClientLight& client)
 throw (std::runtime_error) {
 try
 {
 // Modified by Wei Cao, 2009-02-17
 // Try to find permission from cache first, if no suitable one, request a permission
 // from controller, then store it into cache.
 // TODO: each permission need a TTL
 boost::mutex::scoped_lock lock(permissionCacheMutex_);

 permission_cache_type::iterator it = permissionCache.find(serviceName);
 if(it != permissionCache.end())
 {
 servicePermissionInfo = it->second;
 lock.unlock();
 }
 else
 {
 lock.unlock();
 
 if( false == client.getPermissionOfService(serviceName, servicePermissionInfo) )
 throw std::runtime_error( "failed to get service permission" );

 lock.lock();
 permissionCache.insert(permission_cache_element(serviceName, servicePermissionInfo));
 lock.unlock();			
 }

 //servicePermissionInfo.display();
 }
 catch (MessageFrameworkException& e)
 {
 e.output(std::cout);
 return false;
 }
 return true;
 }*/

/**
 * @brief Process one request at a time.
 */
bool requestService(const string& name, ServiceRequestInfoPtr parameter,
		MessageClientLight& client) throw (std::runtime_error) {

	if (parameter->getServiceName().empty() ) {
		parameter->setServiceName(name);
	} else if (name != parameter->getServiceName() ) {
		cerr << "requestService() failure: Wrong service name" << endl;
		return false;
	}

	try
	{
		runMessageClientLight(parameter, client);
	}
	catch( std::runtime_error & e )
	{
		cerr << "requestService() failure: " << e.what() << endl;
		return false;
	}
	catch (MessageFrameworkException& e)
	{
		e.output(std::cout);
		return false;
	}

	return true;
}

bool requestService(const std::string & name, ServiceRequestInfoPtr parameter,
		ServiceResultPtr & result, MessageClientLight & client)
		throw (std::runtime_error) {
	//result->clear();

	if (parameter->getServiceName().empty() ) {
		parameter->setServiceName(name);
	} else if (name != parameter->getServiceName() ) {
		cerr << "requestService() failure: Wrong service name" << endl;
		return false;
	}

	try
	{
		runMessageClientLight(parameter, result, client);
	}
	catch( std::runtime_error & e )
	{
		cerr << "requestService() failure: " << e.what() << endl;
		return false;
	}
	catch (MessageFrameworkException& e)
	{
		e.output(std::cout);
		return false;
	}

	return true;
}

void runMessageClientLight(ServiceRequestInfoPtr& serviceRequestInfo,
		MessageClientLight& client) throw (std::runtime_error) {
	//ServicePermissionInfo servicePermissionInfo;

	//posix_time::ptime before = posix_time::microsec_clock::local_time();
	///getServicePermissionInfo(serviceName, servicePermissionInfo, client);
	//posix_time::ptime after = posix_time::microsec_clock::local_time();
	//permission_time += (after-before).total_microseconds();

	try
	{
		std::string serviceName = serviceRequestInfo->getServiceName();
		std::map<std::string, MessageFrameworkNode> agentInfoMap;

		if( client.getHostsOfService(serviceName, agentInfoMap) ) {

			std::map<std::string, MessageFrameworkNode>::const_iterator cit = agentInfoMap.begin();
			
			cit->second.display();

			for(; cit != agentInfoMap.end(); cit++) {
				//before = posix_time::microsec_clock::local_time();

				if( false == client.putServiceRequest(cit->second, serviceRequestInfo) )
				throw std::runtime_error( "failed to send service request" );
				//if( servicePermissionInfo.getServiceResultFlag() == SERVICE_WITHOUT_RESULT )
				//    return;
				//after = posix_time::microsec_clock::local_time();
				//send_time += (after-before).total_microseconds();
			}
		} else
		{
			throw std::runtime_error( "failed to send service permission" );
		}
	}
	catch (MessageFrameworkException& e)
	{
		e.output(std::cout);
	}
}

void runMessageClientLight(ServiceRequestInfoPtr& serviceRequestInfo,
		ServiceResultPtr& serviceResult, MessageClientLight& client)
		throw (std::runtime_error) {
	//	std::string serviceName = serviceRequestInfo->getServiceName();
	//	ServicePermissionInfo servicePermissionInfo;

	//	posix_time::ptime before = posix_time::microsec_clock::local_time();
	//	getServicePermissionInfo(serviceName, servicePermissionInfo, client);
	//	posix_time::ptime after = posix_time::microsec_clock::local_time();
	//	permission_time += (after-before).total_microseconds();

	try
	{
		std::string serviceName = serviceRequestInfo->getServiceName();
		std::map<std::string, MessageFrameworkNode> agentInfoMap;
		if( client.getHostsOfService(serviceName, agentInfoMap) ) {

			std::map<std::string, MessageFrameworkNode>::const_iterator cit = agentInfoMap.begin();
			for(; cit != agentInfoMap.end(); cit++) {
				//before = posix_time::microsec_clock::local_time();
				if( false == client.putServiceRequest(cit->second, serviceRequestInfo) )
				throw std::runtime_error( "failed to send service request" );
				//if( servicePermissionInfo.getServiceResultFlag() == SERVICE_WITHOUT_RESULT )
				//    return;
				//after = posix_time::microsec_clock::local_time();
				//send_time += (after-before).total_microseconds();

				//before = posix_time::microsec_clock::local_time();
				if ( false == client.getResultOfService(serviceRequestInfo, serviceResult) )
				throw std::runtime_error( "failed to get service reply" );

				//after = posix_time::microsec_clock::local_time();
				//recv_time += (after-before).total_microseconds();
			}
		}
	}
	catch (MessageFrameworkException& e)
	{
		e.output(std::cout);
	}
}

/**
 * @brief Batch processing requests.
 * Send all requests out at the same time, then wait for their replies
 * Added by Wei Cao, 2009-02-19
 */
bool requestService(const std::string& serviceName,
		std::vector<ServiceRequestInfoPtr>& serviceRequestInfos,
		MessageClientLight& client) throw (std::runtime_error) {
//	ServicePermissionInfo servicePermissionInfo;

//	posix_time::ptime before = posix_time::microsec_clock::local_time();
//	getServicePermissionInfo(serviceName, servicePermissionInfo, client);
//	posix_time::ptime after = posix_time::microsec_clock::local_time();
//	permission_time += (after-before).total_microseconds();

	//servicePermissionInfo.display();

	try
	{
		std::map<std::string, MessageFrameworkNode> agentInfoMap;
		client.getHostsOfService(serviceName, agentInfoMap);
		std::map<std::string, MessageFrameworkNode>::const_iterator cit = agentInfoMap.begin();
		for(; cit != agentInfoMap.end(); cit++) {
			//before = posix_time::microsec_clock::local_time();
			if( false == client.putServiceRequest(cit->second, serviceRequestInfos) )
			throw std::runtime_error( "failed to send service request" );
			//if( servicePermissionInfo.getServiceResultFlag() != SERVICE_WITHOUT_RESULT )
			//	throw std::runtime_error( "call a service with result without waiting for the result" );
			//after = posix_time::microsec_clock::local_time();
			//send_time += (after-before).total_microseconds();
		}
	}
	catch (MessageFrameworkException& e)
	{
		e.output(std::cout);
		return false;
	}
	return true;
}

bool requestService(const std::string& serviceName,
		std::vector<ServiceRequestInfoPtr>& serviceRequestInfos,
		std::vector<ServiceResultPtr>& serviceResults, MessageClientLight& client)
		throw (std::runtime_error) {
	//ServicePermissionInfo servicePermissionInfo;

	//posix_time::ptime before = posix_time::microsec_clock::local_time();
	//getServicePermissionInfo(serviceName, servicePermissionInfo, client);
	//posix_time::ptime after = posix_time::microsec_clock::local_time();
	//permission_time += (after-before).total_microseconds();

	try
	{
		std::map<std::string, MessageFrameworkNode> agentInfoMap;
				client.getHostsOfService(serviceName, agentInfoMap);				
				std::map<std::string, MessageFrameworkNode>::const_iterator cit = agentInfoMap.begin();
		for(; cit != agentInfoMap.end(); cit++) {
			//before = posix_time::microsec_clock::local_time();
			if( false == client.putServiceRequest(cit->second, serviceRequestInfos) )
			throw std::runtime_error( "failed to send service request" );
			//if( servicePermissionInfo.getServiceResultFlag() == SERVICE_WITHOUT_RESULT )
			//    return true;
			//after = posix_time::microsec_clock::local_time();
			//send_time += (after-before).total_microseconds();

			serviceResults.resize(serviceRequestInfos.size());

			//before = posix_time::microsec_clock::local_time();
			if ( false == client.getResultOfService(serviceRequestInfos, serviceResults) )
			throw std::runtime_error( "failed to get service reply" );
			//after = posix_time::microsec_clock::local_time();
			//recv_time += (after-before).total_microseconds();
		}
	}
	catch (MessageFrameworkException& e)
	{
		e.output(std::cout);
		return false;
	}
	return true;
}
}
