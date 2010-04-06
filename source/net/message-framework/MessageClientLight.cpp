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
		return ret;
	}


	bool MessageClientLight::getPermissionOfService(const std::string& serviceName,
			std::map<std::string, MessageFrameworkNode>& servers) {
		ServicePermissionInfo servicePermissionInfo;
		controller_.getServicePermission( serviceName, servicePermissionInfo );
		servers = servicePermissionInfo.getServerMap();
		//cout<<" MessageClientLight::getPermissionOfService "<<endl;
		//servicePermissionInfo.display();
		return true;
	}

	
	bool MessageClientLight::putServiceRequest(const MessageFrameworkNode& server,
						ServiceRequestInfoPtr& serviceRequestInfo, bool withResult){
		unsigned int requestId = generateRequestId();
		serviceRequestInfo->setRequestId( requestId );		
		return controller_.addServiceRequest( server, serviceRequestInfo );
		
	}

	bool MessageClientLight::getResultOfService(const ServiceRequestInfoPtr& serviceRequestInfo,
					ServiceResultPtr& serviceResult){
		 return controller_.getResultByRequestId( serviceRequestInfo->getRequestId(), serviceResult );
	}
	
}
