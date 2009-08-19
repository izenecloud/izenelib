#ifndef LASERVICEHANDLE_H_
#define LASERVICEHANDLE_H_

#include <net/ServiceHandleBase.h>
#include <LAManager.h>

using namespace messageframework;
using namespace sf1v5_dummy;

class LAServiceHandle:public ServiceHandlerBase
{
public:
	void setLAManager(const LAManagerPtr& laMgr)
	{
		laMgr_ = laMgr;		
	}	
	bool parseString(MessageServer& server, ServiceRequestInfoPtr &request)
	{  
		SERVICE_HANDLE_1_1(request, server, std::string, laMgr_->parseString, std::vector<std::string>) 		
	}
private:
	LAManagerPtr laMgr_;
	
};

#endif /*LASERVICEHANDLE_H_*/
