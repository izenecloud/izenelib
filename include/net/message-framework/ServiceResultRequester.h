///
///  @file  MessageClient.h
///  @date  8/5/2008
///  @author  TuanQuang Nguyen
///  @brief This file defines PermissionOfServiceClientHandler class and MessageClient class


#ifndef _SERVICE_RESULT_REQUESTER_H_
#define _SERVICE_RESULT_REQUESTER_H_

#include <net/message-framework/ServiceMessage.h>

/**************** Include std header files *******************/
#include <string>

/**************** Include boost header files *******************/
#include <boost/shared_ptr.hpp>

/**************** Include sf1lib header files *******************/

namespace messageframework {

/**
 * @brief ServiceResultRequester is an abstract class that plays a role of client
 * It request result of a service to ServiceResultServer
 */
class ServiceResultRequester {
public:
	/**
	 * @brief constructor, initialize variables with default values
	 */
	ServiceResultRequester() {
	}

	/**
	 * @brief destructor, destroy variables it if is neccessary
	 */
	virtual ~ServiceResultRequester() {
	}

	/**
	 * @brief This function sends request to the ServiceResultServer
	 * @param
	 * requestId - the id of service request
	 * @param
	 * serviceName - the service name
	 * @param
	 * data - the data of the service request
	 * @param
	 * server - the server that will receive data
	 */

	/*virtual void sendServiceRequest(unsigned requestId,
	 const std::string& serviceName,
	 const std::vector<boost::shared_ptr<VariantType> >& data,
	 const MessageFrameworkNode& server) = 0;*/

	/**
	 * @brief This function is called when a result of serivce arrives. The service result
	 * has been requested before.
	 * @param
	 * requestId - id of the request
	 * @param
	 * serviceName - the name of service
	 * @param
	 * data - result of the service
	 */
	/*virtual void receiveResultOfService(unsigned int requestId,
	 const std::string& serviceName,
	 const std::vector<boost::shared_ptr<VariantType> >& data) = 0;*/

	virtual void sendServiceRequest(const ServiceRequestInfoPtr & requestInfo,
			const MessageFrameworkNode& server) {}

	virtual void receiveResultOfService(const ServiceResultPtr& result) {}
};

}// end of namespace messageframework
#endif  // end of _SERVICE_RESULT_REQUESTER_H_
