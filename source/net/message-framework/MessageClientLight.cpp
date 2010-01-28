/**
 */

#include <net/message-framework/MessageClientLight.h>

namespace messageframework
{
	MessageClientLight::MessageClientLight( const std::string clientName, MessageControllerLight & controller )
		: clientName_(clientName), controller_(controller)
	{
		clientId_ = 0;
		nxtSequentialNumber_ = 0;
	}

	bool MessageClientLight::getClientId(int& clientId)
	{
		if(clientId_ != 0)
		{
			clientId = clientId_;
			return true;
		}

		clientId_ = controller_.applyForClientId();
		if(clientId_ == 0)
			return false;

		clientId = clientId_;
		return true;
	}

	const unsigned int MessageClientLight::generateRequestId() {
		unsigned int ret = 0;
		int clientId;
		if( getClientId(clientId) )
		{
			ret = clientId;
			boost::mutex::scoped_lock lock(nxtSequentialNumberMutex_);
			if( nxtSequentialNumber_ == MF_LIGHT_MAX_SEQUENTIAL_NUMBER )
				nxtSequentialNumber_ = 0;
			ret <<= MF_LIGHT_REQUEST_ID_CLIENT_ID_SHIFT;
			ret |= (nxtSequentialNumber_& MF_LIGHT_MASK_SEQUENTIAL_NUMBER);
			nxtSequentialNumber_ ++;
		}
		//cout<<"MessageClientLight::generateRequestId "<<clientId<<" "<<ret<<endl;
		return ret;
	}

  /*  bool MessageClientLight::getPermissionOfService(const std::string& serviceName,
            ServicePermissionInfo& servicePermissionInfo)
    {
        return controller_.getServicePermission( serviceName, servicePermissionInfo );
    }*/

	bool MessageClientLight::getPermissionOfService(const std::string& serviceName,
			std::map<std::string, MessageFrameworkNode>& servers) {
		ServicePermissionInfo servicePermissionInfo;
		controller_.getServicePermission( serviceName, servicePermissionInfo );
		servers = servicePermissionInfo.getServerMap();
		return true;
	}


    bool MessageClientLight::putServiceRequest(const MessageFrameworkNode& server,
            ServiceRequestInfoPtr& serviceRequestInfo)
    {
    	unsigned int requestId = generateRequestId();
        serviceRequestInfo->setRequestId( requestId );
        //serviceRequestInfo->setServiceResultFlag( servicePermissionInfo.getServiceResultFlag() );
        return controller_.addServiceRequest( server, serviceRequestInfo );
    }

	bool MessageClientLight::putServiceRequest(
			const MessageFrameworkNode& server,
				std::vector<ServiceRequestInfoPtr>& serviceRequestInfos)
	{
		for(unsigned int i=0; i<serviceRequestInfos.size(); i++){
			unsigned int requestId = generateRequestId();
			serviceRequestInfos[i]->setRequestId( requestId );
		}
		return controller_.addServiceRequests( server, serviceRequestInfos );
	}

    bool MessageClientLight::getResultOfService(const ServiceRequestInfoPtr& serviceRequest,
            ServiceResultPtr& serviceResult)
    {
           return controller_.getResultByRequestId( serviceRequest->getRequestId(), serviceResult );
    }

	bool MessageClientLight::getResultOfService(const std::vector<ServiceRequestInfoPtr> & serviceRequests,
				std::vector<ServiceResultPtr> & serviceResults)
	{
		if(serviceRequests.size() == 0)
			return true;
        std::vector<unsigned int> requestIds;
        requestIds.reserve(serviceResults.size() );
		for(unsigned int i=0; i<serviceRequests.size(); i++)
			requestIds.push_back( serviceRequests[i]->getRequestId() );

		return controller_.getResultsByRequestIds(requestIds, serviceResults );
	}
}
