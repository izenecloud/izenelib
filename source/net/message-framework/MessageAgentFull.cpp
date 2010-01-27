///#include <message-framework/Serializable.h>

#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <net/message-framework/MessageAgentFull.h>

using namespace std;
using namespace boost;

namespace messageframework {

/**
 * @brief for profiling - Wei Cao
 */
long permission_time = 0;
long send_time = 0;
long recv_time = 0;

/**
 * @brief Process one request at a time.
 */
bool requestService(const string& name, ServiceRequestInfoPtr parameter,
		MessageClientFull& client) throw (std::runtime_error) {

	if (parameter->getServiceName().empty() ) {
		parameter->setServiceName(name);
	} else if (name != parameter->getServiceName() ) {
		cerr << "requestService() failure: Wrong service name" << endl;
		return false;
	}

	try
	{
		runMessageClientFull(parameter, client);
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
		ServiceResultPtr & result, MessageClientFull & client)
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
		runMessageClientFull(parameter, result, client);
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

void runMessageClientFull(ServiceRequestInfoPtr& serviceRequestInfo,
		MessageClientFull& client) throw (std::runtime_error) {
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

			//cit->second.display();

			for(; cit != agentInfoMap.end(); cit++) {
				//before = posix_time::microsec_clock::local_time();

				if( false == client.putServiceRequest(cit->second, serviceRequestInfo, false) )
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

void runMessageClientFull(ServiceRequestInfoPtr& serviceRequestInfo,
		ServiceResultPtr& serviceResult, MessageClientFull& client)
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
			//assert(agentInfoMap.size() == 1);
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

}
