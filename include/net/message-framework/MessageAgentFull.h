/**
 * @file    MessageAgent.h
 * @brief
 * @author  MyungHyun Lee (kent), orignally by Quang and SeungEun
 * @date    Feb 02, 2009
 */
#ifndef _DOCUMENT_MESSAGE_AGENT_H_
#define _DOCUMENT_MESSAGE_AGENT_H_

#include <net/message-framework/MessageClientFull.h>

namespace messageframework {
/**
 * @brief For profiling, Wei Cao
 */
extern long permission_time;
extern long send_time;
extern long recv_time;

/**
 * @brief Process one request at a time.
 */
// Service with no results
// added @by MyungHyun Lee - Feb 11, 2009
bool requestService(const std::string & name, ServiceRequestInfoPtr parameter,
		MessageClientFull & client) throw (std::runtime_error);

bool requestService(const std::string & name, ServiceRequestInfoPtr parameter,
		ServiceResultPtr & result, MessageClientFull & client)
		throw (std::runtime_error);

/**
 * @brief This set of functions act in a way of "batch processing".
 * Send all requests out at the same time, then wait for their replies
 * Added by Wei Cao, 2009-02-19
 */
bool requestService(const std::string& serviceName,
		std::vector<ServiceRequestInfoPtr>& serviceRequestInfos,
		MessageClientFull& client) throw (std::runtime_error);


bool requestService(const std::string& serviceName,
				std::vector<ServiceRequestInfoPtr>& serviceRequestInfos,
				std::vector<ServiceResultPtr>& serviceResults,
				MessageClientFull& client) throw (std::runtime_error);


bool requestServiceByAgentInfo(const vector<std::string>& agentInfo,
		const std::string & name, ServiceRequestInfoPtr parameter,
		MessageClientFull & client) throw (std::runtime_error);

bool requestServiceByAgentInfo(const vector<std::string>& agentInfo,
		const std::string & name, ServiceRequestInfoPtr parameter,
		ServiceResultPtr & result, MessageClientFull & client)
		throw (std::runtime_error);

bool requestServiceByAgentInfo(const vector<std::string>& agentInfo,
		const std::string& serviceName,
		std::vector<ServiceRequestInfoPtr>& serviceRequestInfos,
		MessageClientFull& client) throw (std::runtime_error);

bool requestServiceByAgentInfo(const vector<std::string>& agentInfo,
				const std::string& serviceName,
				std::vector<ServiceRequestInfoPtr>& serviceRequestInfos,
				std::vector<ServiceResultPtr>& serviceResults,
				MessageClientFull& client) throw (std::runtime_error);


////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Get the service permission info of a given service name
 * from cache if hit, or get it from controller.
 */
bool getServicePermissionInfo(const std::string& serviceName,
				ServicePermissionInfo& servicePermissionInfo,
				MessageClientFull& client) throw (std::runtime_error);

bool getServiceServer(const std::vector<std::string>& agentInfos, const std::string& serviceName,		       
				  vector<MessageFrameworkNode>& servers, const MessageClientFull& client) throw (std::runtime_error);


bool getServiceServer(const std::string& serviceName,		       
				  vector<MessageFrameworkNode>& servers, const MessageClientFull& client) throw (std::runtime_error);


void runMessageClientFull(ServiceRequestInfoPtr& serviceRequestInfo,
		ServiceResultPtr& serviceResult, MessageClientFull& client)
		throw (std::runtime_error);

void runMessageClientFull(ServiceRequestInfoPtr& serviceRequestInfo,
		MessageClientFull& client) throw (std::runtime_error);

}

#endif
