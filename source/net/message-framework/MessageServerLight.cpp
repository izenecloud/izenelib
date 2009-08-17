/**
 */

#include <net/message-framework/MessageServerLight.h>


namespace messageframework
{
  /*  bool MessageServerLight::registerService( 
            const std::string & serviceName,
            const std::vector<messageframework::ServiceParameterType> & parameterTypeList, 
            const messageframework::PermissionFlag & permissionFlag,
            const ServiceResultFlag serviceResultFlag 
            )
    {
        ServiceInfo serviceInfo;

        MessageFrameworkNode server;
        server.nodeName_ = serverName_;

        serviceInfo.setServer( server );
        serviceInfo.setServiceName( serviceName );
        serviceInfo.setParameterList( parameterTypeList );
        serviceInfo.setPermissionFlag( permissionFlag );
        serviceInfo.setServiceResultFlag( serviceResultFlag );
        //TODO:serviceInfo.setServer();

        {
            //cout << "SERVER:\t " << serviceInfo.getServer().nodeName_ << endl;
        }
        return controller_.addServiceRegistrationRequest( serviceInfo );
    }*/

    bool MessageServerLight::registerService( const ServiceInfo & serviceInfo )
    {
		ServiceRegistrationMessage message;
		message.setAgentInfo(agentInfo_);
		message.setServiceInfo(serviceInfo);
		
    	return controller_.addServiceRegistrationRequest( message );
    }


    bool MessageServerLight::getServiceRequestList( std::vector<ServiceRequestInfoPtr> & requestList )
    {
        //return controller_.getRequestListByServer( serverName_, requestList );
        return controller_.getRequestListByServer( thisNode_, requestList );
    }


    
    bool MessageServerLight::putResultOfService(            
            const ServiceResultPtr & result )
    {
        //the service provides no return value  @by MyungHyun - 2009-01-28
       /* if( serviceRequestInfo.getServiceResultFlag() ==  messageframework::SERVICE_WITHOUT_RESULT)
        {
            return false;
        }*/

        return controller_.addResultOfService( result );
    }
}
