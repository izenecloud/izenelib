#ifndef _MESSAGE_TYPE_H_
#define _MESSAGE_TYPE_H_

namespace messageframework
{
	enum MessageType
	{
			// Fist message to be sent right after connection to peer node is successful
			HELLO_MSG,
			// Message to register a service
			SERVICE_REGISTRATION_REQUEST_MSG,
			// Message to inform success of service registration
			SERVICE_REGISTRATION_REPLY_MSG,
			// Message to retrieve a service permission
			PERMISSION_OF_SERVICE_REQUEST_MSG,
			// Message to inform success of permission retrieval
			PERMISSION_OF_SERVICE_REPLY_MSG,  
			// Message to request a service result
			SERVICE_REQUEST_MSG,
			// Message that contains service result
			SERVICE_RESULT_MSG,			
			//SERVICE_MSG,			
			UNKNOWN_MSG,
			    // Message to get a client id
			CLIENT_ID_REQUEST_MSG,
			// Message to send back a client id
			CLIENT_ID_REPLY_MSG,
	};
}// end of namespace messageframework

#endif
