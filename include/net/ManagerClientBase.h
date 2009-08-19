
#ifndef _MANAGER_CLIENT_BASE_H_
#define _MANAGER_CLIENT_BASE_H_

#include <net/message_framework.h>
#include <net/message-framework/MessageAgent.h>

#include <util/log.h>

#include <cache/MLRUCache.h>

using namespace izenelib::cache;
using namespace messageframework;


#define CLIENT_REQUEST_IMPL_0_1(servicename, MessageClient, result) \
	ServiceRequestInfoPtr req(new ServiceRequestInfo); \
	ServiceResultPtr res; \
	req->setServiceName( servicename ); \
    if ( requestService(servicename, req, res, MessageClient) ) \
    { \
	  	  mf_deserialize(result, res); \
	      return true; \
	} \
    return false; \
    

#define CLIENT_REQUEST_IMPL_0_0(servicename, MessageClient) \
	ServiceRequestInfoPtr req(new ServiceRequestInfo); \
	req->setServiceName( servicename ); \
    if ( requestService(servicename, req,  MessageClient) ) \
    { \
	     return true; \
	} \
    return false; \


#define CLIENT_REQUEST_IMPL_1_1(servicename, MessageClient, param1, result) \
	ServiceRequestInfoPtr req(new ServiceRequestInfo); \
	ServiceResultPtr res; \
	req->setServiceName( servicename ); \
	mf_serialize(param1, req); \
    if ( requestService(servicename, req, res, MessageClient) ) \
    { \
	  	  mf_deserialize(result, res); \
	      return true; \
	} \
    return false; \
    

#define CLIENT_REQUEST_IMPL_1_0(servicename, MessageClient, param1) \
	ServiceRequestInfoPtr req(new ServiceRequestInfo); \
	req->setServiceName( servicename ); \
	mf_serialize(param1, req); \
    if ( requestService(servicename, req, MessageClient) ) \
    { \
	      return true; \
	} \
    return false; \
    

#define CLIENT_REQUEST_IMPL_2_1(servicename, MessageClient, param1, param2, result) \
	ServiceRequestInfoPtr req(new ServiceRequestInfo); \
	ServiceResultPtr res; \
	req->setServiceName( servicename ); \
	mf_serialize(param1, req, 0); \
	mf_serialize(param2, req, 1); \
    if ( requestService(servicename, req, res, MessageClient) ) \
    { \
    	mf_deserialize(result, res); \
    	return true; \
	} \
    return false; \
    
    
#define CLIENT_REQUEST_IMPL_2_0(servicename, MessageClient, param1, param2) \
	ServiceRequestInfoPtr req(new ServiceRequestInfo); \
	req->setServiceName( servicename ); \
	mf_serialize(param1, req, 0); \
	mf_serialize(param2, req, 1); \
    if ( requestService(servicename, req, MessageClient) ) \
    { \
	      return true; \
	} \
    return false; \
    
    
    
#define CLIENT_REQUEST_IMPL_3_1(servicename, MessageClient, param1, param2, param3, result) \
	ServiceRequestInfoPtr req(new ServiceRequestInfo); \
	ServiceResultPtr res; \
	req->setServiceName( servicename ); \
	mf_serialize(param1, req, 0); \
	mf_serialize(param2, req, 1); \
	mf_serialize(param3, req, 2); \
    if ( requestService(servicename, req, res, MessageClient) ) \
    { \
	  	  mf_deserialize(result, res); \
	      return true; \
	} \
    return false; \
    
#define CLIENT_REQUEST_IMPL_4_1(servicename, MessageClient, param1, param2, param3, param4, result) \
	ServiceRequestInfoPtr req(new ServiceRequestInfo); \
	ServiceResultPtr res; \
	req->setServiceName( servicename ); \
	mf_serialize(param1, req, 0); \
	mf_serialize(param2, req, 1); \
	mf_serialize(param3, req, 2); \
	mf_serialize(param4, req, 3); \
    if ( requestService(servicename, req, res, MessageClient) ) \
    { \
	  	  mf_deserialize(result, res); \
	      return true; \
	} \
    return false; \
    
#define CLIENT_REQUEST_IMPL_5_1(servicename, MessageClient, param1, param2, param3, param4, param5, result) \
	ServiceRequestInfoPtr req(new ServiceRequestInfo); \
	ServiceResultPtr res; \
	req->setServiceName( servicename ); \
	mf_serialize(param1, req, 0); \
	mf_serialize(param2, req, 1); \
	mf_serialize(param3, req, 2); \
	mf_serialize(param4, req, 3); \
	mf_serialize(param5, req, 4); \
    if ( requestService(servicename, req, res, MessageClient) ) \
    { \
	  	  mf_deserialize(result, res); \
	      return true; \
	} \
    return false; \
    
    
#define CLIENT_REQUEST_IMPL_6_1(servicename, MessageClient, param1, param2, param3, param4, param5, param6, result) \
	ServiceRequestInfoPtr req(new ServiceRequestInfo); \
	ServiceResultPtr res; \
	req->setServiceName( servicename ); \
	mf_serialize(param1, req, 0); \
	mf_serialize(param2, req, 1); \
	mf_serialize(param3, req, 2); \
	mf_serialize(param4, req, 3); \
	mf_serialize(param5, req, 4); \
	mf_serialize(param6, req, 5); \
    if ( requestService(servicename, req, res, MessageClient) ) \
    { \
	  	  mf_deserialize(result, res); \
	      return true; \
	} \
    return false; \
    
#define CLIENT_REQUEST_IMPL_4_2(servicename, MessageClient, param1, param2, param3, param4, result1, result2) \
	ServiceRequestInfoPtr req(new ServiceRequestInfo); \
	ServiceResultPtr res; \
	req->setServiceName( servicename ); \
	mf_serialize(param1, req, 0); \
	mf_serialize(param2, req, 1); \
	mf_serialize(param3, req, 2); \
	mf_serialize(param4, req, 3); \
    if ( requestService(servicename, req, res, MessageClient) ) \
    { \
	  	  mf_deserialize(result1, res, 0); \
	  	  mf_deserialize(result2, res, 1); \
	  	  return true; \
	} \
    return false; \
    
    
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
#define CLIENT_REQUEST_IMPL_1_1_CACHE(servicename, MessageClient, param1, result, cache) \
	if(cache.getValue(param1, result) ) \
        return true; \
	ServiceRequestInfoPtr req(new ServiceRequestInfo); \
	ServiceResultPtr res; \
	req->setServiceName( servicename ); \
	mf_serialize(param1, req); \
    if ( requestService(servicename, req, res, MessageClient) ) \
    { \
	  	  mf_deserialize(result, res); \
	  	  cache.insertValue(param1, result); \
	      return true; \
	} \
    return false; \
    
#define CLIENT_REQUEST_IMPL_2_1_CACHE(servicename, MessageClient, param1, param2, result, cache) \
	if(cache.getValue(boost::make_tuple(param1, param2), result) ) \
        return true; \
	ServiceRequestInfoPtr req(new ServiceRequestInfo); \
	ServiceResultPtr res; \
	req->setServiceName( servicename ); \
	mf_serialize(param1, req, 0); \
	mf_serialize(param2, req, 1); \
    if ( requestService(servicename, req, res, MessageClient) ) \
    { \
	  	  mf_deserialize(result, res); \
	  	  cache.insertValue(boost::make_tuple(param1,param2), result); \
	      return true; \
	} \
    return false; \
    
    
#define CLIENT_REQUEST_IMPL_3_1_CACHE(servicename, MessageClient, param1, param2, param3, result, cache) \
	if(cache.getValue(boost::make_tuple(param1, param2, param3), result) ) \
        return true; \
	ServiceRequestInfoPtr req(new ServiceRequestInfo); \
	ServiceResultPtr res; \
	req->setServiceName( servicename ); \
	mf_serialize(param1, req, 0); \
	mf_serialize(param2, req, 1); \
	mf_serialize(param3, req, 2); \
    if ( requestService(servicename, req, res, MessageClient) ) \
    { \
	  	  mf_deserialize(result, res); \
	  	  cache.insertValue(boost::make_tuple(param1, param2, param3), result); \
	      return true; \
	} \
    return false; \
    



#endif // _MANAGER_CLIENT_BASE_H_
