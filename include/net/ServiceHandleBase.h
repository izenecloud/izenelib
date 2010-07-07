#ifndef SERVICEHANDLEBASE_H_
#define SERVICEHANDLEBASE_H_

#include <net/message_framework.h>

namespace messageframework{

class ServiceHandlerBase
{
    public:    
    void no_callback(ServiceRequestInfoPtr& request){        
        	   DLOG(ERROR) << "Could not found callback function: service name=" << request->getServiceName().c_str() << endl;
    }   
};

}

#define SERVICE_HANDLE_0_1(request, server, FUNCTION, TYPE1) \
unsigned int n = request->getBufferNum(); \
if( n==0 ) n = 1; \
for(unsigned int i=0; i<n; i++){ \
	TYPE1 t1; \
	FUNCTION(t1); \
        mf_serialize(t1, request, i); \
} \
return server.putResultOfService(request); \


#define SERVICE_HANDLE_0_2(request, server, FUNCTION, TYPE1, TYPE2) \
	TYPE1 t1; \
        TYPE2 t2; \
	FUNCTION(t1, t2); \
        mf_serialize(t1, request, 0); \
        mf_serialize(t2, request, 1); \
return server.putResultOfService(request); \



#define SERVICE_HANDLE_1_0(request, server, TYPE1, FUNCTION) \
for(unsigned int i=0; i<request->getBufferNum(); i++){ \
	TYPE1 t1; \
    mf_deserialize(t1, request, i); \
	FUNCTION(t1); \
} \



#define SERVICE_HANDLE_2_0(request, server, TYPE1, TYPE2, FUNCTION) \
for(unsigned int i=0; i<request->getBufferNum(); i+=2){ \
	TYPE1 t1; \
	TYPE2 t2; \
    mf_deserialize(t1, request, i); \
    mf_deserialize(t2, request, i+1); \
	FUNCTION(t1, t2); \
} \


#define SERVICE_HANDLE_1_1_R(request, server, TYPE1, FUNCTION, TYPE2) \
for(unsigned int i=0; i<request->getBufferNum(); i++){ \
	TYPE1 t1; \
	TYPE2 t2; \
    mf_deserialize(t1, request, i); \
	t2 = FUNCTION(t1); \
	mf_serialize(t2, request, i); \
} \
return server.putResultOfService(request); \


#define SERVICE_HANDLE_1_1(request, server, TYPE1, FUNCTION, TYPE2) \
for(unsigned int i=0; i<request->getBufferNum(); i++){ \
	TYPE1 t1; \
	TYPE2 t2; \
    mf_deserialize(t1, request, i); \
	FUNCTION(t1, t2); \
	mf_serialize(t2, request, i); \
} \
return server.putResultOfService(request); \


#define SERVICE_HANDLE_2_1(request, server, TYPE1, TYPE2, FUNCTION, TYPE3) \
for(unsigned int i=0; i<request->getBufferNum(); i+=2){ \
	TYPE1 t1; \
	TYPE2 t2; \
	TYPE3 t3; \
	mf_deserialize(t1, request, i); \
    mf_deserialize(t2, request, i+1); \
	FUNCTION(t1, t2, t3); \
	mf_serialize(t3, request, i/2); \
} \
request->setBufferNum(request->getBufferNum()/2); \
return server.putResultOfService(request); \


#define SERVICE_HANDLE_3_1(request, server, TYPE1, TYPE2, TYPE3, FUNCTION, TYPE4) \
for(unsigned int i=0; i<request->getBufferNum(); i+=3){ \
	TYPE1 t1; \
	TYPE2 t2; \
	TYPE3 t3; \
	TYPE4 t4; \
	mf_deserialize(t1, request, i); \
    mf_deserialize(t2, request, i+1); \
    mf_deserialize(t3, request, i+2); \
	FUNCTION(t1, t2, t3, t4); \
	mf_serialize(t4, request, i/3); \
} \
request->setBufferNum(request->getBufferNum()/3); \
return server.putResultOfService(request); \


#define SERVICE_HANDLE_4_1(request, server, TYPE1, TYPE2, TYPE3, TYPE4, FUNCTION, TYPE5) \
for(unsigned int i=0; i<request->getBufferNum(); i+=4){ \
	TYPE1 t1; \
	TYPE2 t2; \
    TYPE3 t3; \
    TYPE4 t4; \
    TYPE5 t5; \
    mf_deserialize(t1, request, i); \
    mf_deserialize(t2, request, i+1); \
    mf_deserialize(t3, request, i+2); \
    mf_deserialize(t4, request, i+3); \
	FUNCTION(t1, t2, t3, t4, t5); \
	mf_serialize(t5, request, i/4); \
} \
request->setBufferNum(request->getBufferNum()/4); \
return server.putResultOfService(request); \



#define SERVICE_HANDLE_5_1(request, server, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, FUNCTION, TYPE6) \
for(unsigned int i=0; i<request->getBufferNum(); i+=5){ \
	TYPE1 t1; \
	TYPE2 t2; \
    TYPE3 t3; \
    TYPE4 t4; \
    TYPE5 t5; \
    TYPE6 t6; \
    mf_deserialize(t1, request, i); \
    mf_deserialize(t2, request, i+1); \
    mf_deserialize(t3, request, i+2); \
    mf_deserialize(t4, request, i+3); \
    mf_deserialize(t5, request, i+4); \
	FUNCTION(t1, t2, t3, t4, t5, t6); \
	mf_serialize(t6, request, i/5); \
} \
request->setBufferNum(request->getBufferNum()/5); \
return server.putResultOfService(request); \



#define SERVICE_HANDLE_6_1(request, server, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, FUNCTION, TYPE7) \
for(unsigned int i=0; i<request->getBufferNum(); i+=6){ \
	TYPE1 t1; \
	TYPE2 t2; \
    TYPE3 t3; \
    TYPE4 t4; \
    TYPE5 t5; \
    TYPE6 t6; \
    TYPE7 t7; \
    mf_deserialize(t1, request, i); \
    mf_deserialize(t2, request, i+1); \
    mf_deserialize(t3, request, i+2); \
    mf_deserialize(t4, request, i+3); \
    mf_deserialize(t5, request, i+4); \
    mf_deserialize(t6, request, i+5); \
	FUNCTION(t1, t2, t3, t4, t5, t6, t7); \
	mf_serialize(t7, request, i/6); \
} \
request->setBufferNum(request->getBufferNum()/6); \
return server.putResultOfService(request); \
	


#define SERVICE_HANDLE_4_2(request, server, TYPE1, TYPE2, TYPE3, TYPE4, FUNCTION, TYPE5, TYPE6) \
for(unsigned int i=0; i<request->getBufferNum(); i+=4){ \
	TYPE1 t1; \
	TYPE2 t2; \
    TYPE3 t3; \
    TYPE4 t4; \
    TYPE5 t5; \
    TYPE6 t6; \
    mf_deserialize(t1, request, i); \
    mf_deserialize(t2, request, i+1); \
    mf_deserialize(t3, request, i+2); \
    mf_deserialize(t4, request, i+3); \
	FUNCTION(t1, t2, t3, t4, t5, t6); \
	mf_serialize(t5, request, i/4); \
	mf_serialize(t6, request, i/4+1); \
} \
request->setBufferNum(request->getBufferNum()/4*2); \
return server.putResultOfService(request); \
	

#define SERVICE_HANDLE_5_2(request, server, TYPE1, TYPE2, TYPE3, TYPE4, TYPE5, FUNCTION, TYPE6, TYPE7) \
for(unsigned int i=0; i<request->getBufferNum(); i+=4){ \
	TYPE1 t1; \
	TYPE2 t2; \
    TYPE3 t3; \
    TYPE4 t4; \
    TYPE5 t5; \
    TYPE6 t6; \
    TYPE7 t7; \
    mf_deserialize(t1, request, i); \
    mf_deserialize(t2, request, i+1); \
    mf_deserialize(t3, request, i+2); \
    mf_deserialize(t4, request, i+3); \
    mf_deserialize(t5, request, i+4); \
	FUNCTION(t1, t2, t3, t4, t5, t6, t7); \
	mf_serialize(t5, request, i/5); \
	mf_serialize(t6, request, i/5+1); \
} \
request->setBufferNum(request->getBufferNum()/5*2); \
return server.putResultOfService(request); \





#endif /*SERVICEHANDLEBASE_H_*/
