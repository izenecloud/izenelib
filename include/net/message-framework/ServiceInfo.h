///
///  @file : ServiceInfo.h
///  @date : 8/5/2008
///  @author : TuanQuang Nguyen
///  ==============================
///
///  @date : 8/17/2009
///  @author : Peisheng Wang
///


#if !defined(_SERVICEINFO_H)
#define _SERVICEINFO_H

#include <net/message-framework/MessageFrameworkNode.h>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

namespace messageframework {
/**
 * @brief This class defines a service. It consists of a service name, and its list of
 * parameter types.
 */
class ServiceInfo {
public:
	/**
	 * @brief constructor, initilize variables with default values
	 */
	ServiceInfo();
	
	ServiceInfo(const std::string& serviceName);

	ServiceInfo(const ServiceInfo& inputService);

	/**
	 * @brief destructor, destroy variables if it is neccessary
	 */
	~ServiceInfo();

	/**
	 * @brief clear internal data
	 */
	void clear();

	/**
	 * @brief Set server information
	 * @param
	 * serverIP - the IP of the server
	 * @param
	 * serverPort - the port of the server
	 */
	void setServer(const std::string& serverIP, unsigned int serverPort);

	// ADDED @by MyungHyun (Kent) -2008-11-27
	void setServer(const MessageFrameworkNode & server);

	/**
	 * @brief Get server information
	 * @return
	 * The information of the server
	 */
	const MessageFrameworkNode& getServer() const;
	
	/**
	 * @brief Set service name
	 * @param
	 * serviceName - new service name
	 */
	void setServiceName(const std::string& serviceName);

	/**
	 * @brief Set service name
	 * @return 
	 *  the service name
	 */
	const std::string& getServiceName() const;


	template <typename Archive> void serialize(Archive & ar,
			const unsigned int version) {
		//ar & server_ & serviceName_ & permissionFlag_ & parameterList_
		//		& serviceResultFlag_;
		ar & server_ & serviceName_;
	}

	void display(std::ostream &os=std::cout) const {
		os<<"ServiceInfo: ";
		os<<serviceName_<<std::endl;
		os<<server_.nodeName_<<std::endl;
	}

private:
	/**
	 * @brief The server of the service, it consists of an IP and a port number
	 */
	MessageFrameworkNode server_;

	/**
	 * @brief The service name
	 */
	std::string serviceName_;
	
};

typedef boost::shared_ptr<ServiceInfo> ServiceInfoPtr;

}// end of messageframework
#endif  //_SERVICEINFO_H
