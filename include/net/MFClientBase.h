#ifndef _MFCLIENT_BASE_H_
#define _MFCLIENT_BASE_H_

#include <net/message_framework.h>
#include <util/log.h>
#include <cache/MLRUCache.h>

using namespace izenelib::cache;
using namespace messageframework;


/*
template<typename TYPE >
void mf_client_deserialize(TYPE& result, vector<ServiceResultPtr>& res){
	assert(res.size() == 1);
	for(unsigned int i=0; i<res.size(); i++){
		mf_deserialize(result, res[i]);
	}	
}*/


template<typename TYPE >
void mf_client_deserialize(std::vector<TYPE>& result, std::vector<ServiceResultPtr>& res){
	result.resize(res.size() );
	for(unsigned int i=0; i<res.size(); i++){
		mf_deserialize(result[i], res[i]);
	}	
}



#define MF_CLIENT_IMPL_0_0(agentInfos, servicename) \
	ServiceRequestInfoPtr req(new ServiceRequestInfo); \
	req->setServiceName( servicename ); \
    if ( requestService(agentInfos, servicename, req) ) \
    { \
	     return true; \
	} \
    return false; \


#define MF_CLIENT_IMPL_0_1(agentInfos, servicename, result) \
	ServiceRequestInfoPtr req(new ServiceRequestInfo); \
	std::vector<ServiceResultPtr> res; \
	result.resize(res.size() ); \
	req->setServiceName( servicename ); \
    if ( requestService(agentInfos, servicename, req, res) ) \
    { \
	  	  mf_client_deserialize(result, res); \
	      return true; \
	} \
    return false; \
    
   

#define MF_CLIENT_IMPL_1_0(agentInfos, servicename, param1) \
	ServiceRequestInfoPtr req(new ServiceRequestInfo); \
	req->setServiceName( servicename ); \
	mf_serialize(param1, req); \
    if ( requestService(agentInfos, servicename, req) ) \
    { \
	      return true; \
	} \
    return false; \
    

#define MF_CLIENT_IMPL_1_1(agentInfos, servicename, param1, result) \
	ServiceRequestInfoPtr req(new ServiceRequestInfo); \
	std::vector<ServiceResultPtr> res; \
	req->setServiceName( servicename ); \
	mf_serialize(param1, req); \
    if ( requestService(agentInfos, servicename, req, res) ) \
    { \
	  	  mf_client_deserialize(result, res); \
	      return true; \
	} \
    return false; \
    
    
#define MF_CLIENT_IMPL_2_0(agentInfos, servicename, param1, param2) \
	ServiceRequestInfoPtr req(new ServiceRequestInfo); \
	req->setServiceName( servicename ); \
	mf_serialize(param1, req, 0); \
	mf_serialize(param2, req, 1); \
    if ( requestService(agentInfos, servicename, req) ) \
    { \
	      return true; \
	} \
    return false; \
    
    

#define MF_CLIENT_IMPL_2_1(agentInfos, servicename, param1, param2, result) \
	ServiceRequestInfoPtr req(new ServiceRequestInfo); \
	std::vector<ServiceResultPtr> res; \
	req->setServiceName( servicename ); \
	mf_serialize(param1, req, 0); \
	mf_serialize(param2, req, 1); \
    if ( requestService(agentInfos, servicename, req, res) ) \
    { \
    	mf_client_deserialize(result, res); \
    	return true; \
	} \
    return false; \
    
  
    
    
#define MF_CLIENT_IMPL_3_1(agentInfos, servicename, param1, param2, param3, result) \
	ServiceRequestInfoPtr req(new ServiceRequestInfo); \
	std::vector<ServiceResultPtr> res; \
	req->setServiceName( servicename ); \
	mf_serialize(param1, req, 0); \
	mf_serialize(param2, req, 1); \
	mf_serialize(param3, req, 2); \
    if ( requestService(agentInfos, servicename, req, res) ) \
    { \
	  	  mf_client_deserialize(result, res); \
	      return true; \
	} \
    return false; \
    
#define MF_CLIENT_IMPL_4_1(agentInfos, servicename, param1, param2, param3, param4, result) \
	ServiceRequestInfoPtr req(new ServiceRequestInfo); \
	std::vector<ServiceResultPtr> res; \
	req->setServiceName( servicename ); \
	mf_serialize(param1, req, 0); \
	mf_serialize(param2, req, 1); \
	mf_serialize(param3, req, 2); \
	mf_serialize(param4, req, 3); \
    if ( requestService(agentInfos, servicename, req, res) ) \
    { \
	  	  mf_client_deserialize(result, res); \
	      return true; \
	} \
    return false; \
    
#define MF_CLIENT_IMPL_5_1(agentInfos, servicename, param1, param2, param3, param4, param5, result) \
	ServiceRequestInfoPtr req(new ServiceRequestInfo); \
	std::vector<ServiceResultPtr> res; \
	req->setServiceName( servicename ); \
	mf_serialize(param1, req, 0); \
	mf_serialize(param2, req, 1); \
	mf_serialize(param3, req, 2); \
	mf_serialize(param4, req, 3); \
	mf_serialize(param5, req, 4); \
    if ( requestService(agentInfos, servicename, req, res) ) \
    { \
	  	  mf_client_deserialize(result, res); \
	      return true; \
	} \
    return false; \
    
    
#define MF_CLIENT_IMPL_6_1(agentInfos, servicename, param1, param2, param3, param4, param5, param6, result) \
	ServiceRequestInfoPtr req(new ServiceRequestInfo); \
	std::vector<ServiceResultPtr> res; \
	req->setServiceName( servicename ); \
	mf_serialize(param1, req, 0); \
	mf_serialize(param2, req, 1); \
	mf_serialize(param3, req, 2); \
	mf_serialize(param4, req, 3); \
	mf_serialize(param5, req, 4); \
	mf_serialize(param6, req, 5); \
    if ( requestService(agentInfos, servicename, req, res) ) \
    { \
	  	  mf_client_deserialize(result, res); \
	      return true; \
	} \
    return false; \
    
/*
#define MF_CLIENT_IMPL_4_2(agentInfos, servicename, param1, param2, param3, param4, result1, result2) \
	ServiceRequestInfoPtr req(new ServiceRequestInfo); \
	ServiceResultPtr res; \
	req->setServiceName( servicename ); \
	mf_serialize(param1, req, 0); \
	mf_serialize(param2, req, 1); \
	mf_serialize(param3, req, 2); \
	mf_serialize(param4, req, 3); \
    if ( requestService(agentInfos, servicename, req, res) ) \
    { \
	  	  mf_deserialize(result1, res, 0); \
	  	  mf_deserialize(result2, res, 1); \
	  	  return true; \
	} \
    return false; \
    */
    
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
#define MF_CLIENT_IMPL_1_1_CACHE(agentInfos, servicename, param1, result, cache) \
	if(cache.getValue(param1, result) ) \
        return true; \
	ServiceRequestInfoPtr req(new ServiceRequestInfo); \
	std::vector<ServiceResultPtr> res; \
	req->setServiceName( servicename ); \
	mf_serialize(param1, req); \
    if ( requestService(agentInfos, servicename, req, res) ) \
    { \
	  	  mf_client_deserialize(result, res); \
	  	  cache.insertValue(param1, result); \
	      return true; \
	} \
    return false; \
    
#define MF_CLIENT_IMPL_2_1_CACHE(agentInfos, servicename, param1, param2, result, cache) \
	if(cache.getValue(boost::make_tuple(param1, param2), result) ) \
        return true; \
	ServiceRequestInfoPtr req(new ServiceRequestInfo); \
	std::vector<ServiceResultPtr> res; \
	req->setServiceName( servicename ); \
	mf_serialize(param1, req, 0); \
	mf_serialize(param2, req, 1); \
    if ( requestService(agentInfos, servicename, req, res) ) \
    { \
	  	  mf_client_deserialize(result, res); \
	  	  cache.insertValue(boost::make_tuple(param1,param2), result); \
	      return true; \
	} \
    return false; \
    
    
#define MF_CLIENT_IMPL_3_1_CACHE(agentInfos, servicename, param1, param2, param3, result, cache) \
	if(cache.getValue(boost::make_tuple(param1, param2, param3), result) ) \
        return true; \
	ServiceRequestInfoPtr req(new ServiceRequestInfo); \
	std::vector<ServiceResultPtr> res; \
	req->setServiceName( servicename ); \
	mf_serialize(param1, req, 0); \
	mf_serialize(param2, req, 1); \
	mf_serialize(param3, req, 2); \
    if ( requestService(agentInfos, servicename, req, res) ) \
    { \
	  	  mf_client_deserialize(result, res); \
	  	  cache.insertValue(boost::make_tuple(param1, param2, param3), result); \
	      return true; \
	} \
    return false; \
    

#endif

