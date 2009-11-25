#ifndef _MESSAGE_FRAMEWORK_CONFIGURATION_H
#define _MESSAGE_FRAMEWORK_CONFIGURATION_H


namespace messageframework
{

	// This port is used by MessageController to serve requests that
	// retrieves permission of services
	#define SERVICE_PERMISSION_RETRIEVAL_PORT 9000

	// This port is used by MessageController to serve requests that
	// register services of MessageServer
	#define SERVICE_REGISTRATION_PORT 9001

	// This port is used by MessageController to serve client requests
	// that retrieves results of services
	#define SERVICE_PORT_AT_CONTROLLER 9003

	#define SERVICE_PORT_AT_SERVER  7002

	// This port is used by IndexProcess to serve all request related to
	// IndexManager
	#define INDEX_PROCESS_SERVICE_PORT_AT_SERVER  9010

	// if the data has not arrived for TIME_OUT_IN_MILISECOND, the connection
	// will be automatically disconnected
	
	#define TIME_OUT_IN_MILISECOND 5000*2

	#define INVALID_MESSAGE_ID 0
} // end of namespace messageframework

#endif
