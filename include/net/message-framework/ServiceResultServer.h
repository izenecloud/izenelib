///
///  @file  ServiceResultServer.h
///  @date  8/5/2008
///  @author  TuanQuang Nguyen
///  @brief This file defines ServiceResultServer class
///


#ifndef _SERVICE_RESULT_SERVER_H_
#define _SERVICE_RESULT_SERVER_H_



/**************** Include module header files *******************/
#include <net/message-framework/MessageFrameworkNode.h>
#include <net/message-framework/ServiceMessage.h>

/**************** Include std header files *******************/
#include <string>

/**************** Include boost header files *******************/
#include <boost/shared_ptr.hpp>

/**************** Include sf1lib header files *******************/


namespace messageframework
{

	/**
	 * @brief ServiceResultServer is an abstract class that plays a role of server.
	 * It serves result of a service.
	 */
	class ServiceResultServer
	{
	public:
		/**
		 * @brief constructor, initialize variables with default values
		 */
		ServiceResultServer(){}

		/**
		 * @brief destructor, destroy variables it if is neccessary
		 */
	 	virtual ~ServiceResultServer(){}

		/**
 		 * @brief This function put the request for a service result in the waiting list.
 	 	 * @param requester requester that requests for result
 		 * @param requestId the id of the request
 		 * @param serviceName the service name
 		 * @param data the parameter data of the request
 		 */
		/*virtual void receiveServiceRequest(const MessageFrameworkNode& requester,
						unsigned int requestId,
						const std::string& serviceName,
						const std::vector<boost::shared_ptr<VariantType> >& data) = 0;*/
			
		/**
		 * @brief This function replies to a request from ServiceResultRequester. I will sends
		 * ServiceResult to the ServiceResultRequester.
		 * @param
		 * requester - the requester that requests for result
		 * @param
		 * requestInfo - information of the request
		 * @param
		 * result - result of the requested service
		 */
		/*virtual void sendResultOfService(const MessageFrameworkNode& requester,
				const ServiceRequestInfo& requestInfo,
				const ServiceResult& result) = 0;*/

		virtual void receiveServiceRequest(const MessageFrameworkNode& requester,
						ServiceRequestInfoPtr & requestInfo) {}
		
		virtual void sendResultOfService(const MessageFrameworkNode& requester,				
				const ServiceResultPtr& result) {}

		

	};

}// end of namespace messageframework
#endif  // end of _SERVICE_RESULT_REQUESTER_H_
