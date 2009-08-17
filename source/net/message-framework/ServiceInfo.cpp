///
///  @file : ServiceInfo.cpp
///  @date : 8/5/2008
///  @author : TuanQuang Nguyen
///

#include <net/message-framework/ServiceInfo.h>

#include <string>
#include <vector>


namespace messageframework
{
		/**
		 * @brief constructor, initilize variables with default values
		 */
		ServiceInfo::ServiceInfo()
		{
			server_.nodeIP_ = "";
			server_.nodePort_ = 0;
			//serviceName_.clear();
			//parameterList_.clear();
			//permissionFlag_ = UNKNOWN_PERMISSION_FLAG;
            //erviceResultFlag_ = SERVICE_WITH_RESULT;
		}

		ServiceInfo::ServiceInfo(const ServiceInfo& inputService)
		{
			server_ = inputService.getServer();	
			serviceName_ = inputService.getServiceName();
			//inputService.getParameterList(parameterList_);
			//permissionFlag_ = inputService.getPermissionFlag();
            //serviceResultFlag_ = inputService.getServiceResultFlag();
		}

		/**
		 * @brief destructor, destroy variables if it is neccessary
		 */
		ServiceInfo::~ServiceInfo()
		{
		}
		/**
 		 * @brief clear internal data
 		 */
		void ServiceInfo::clear()
		{
			server_.clear();
			//serviceName_.clear();
			//parameterList_.clear();
			//permissionFlag_ = UNKNOWN_PERMISSION_FLAG;
            //serviceResultFlag_ = SERVICE_WITH_RESULT;
		}

        /* @by MyungHyun - 2009-01-28
		ServiceInfo& ServiceInfo::operator=(const ServiceInfo& serviceInfo)
		{
			serviceName_ = serviceInfo.getServiceName();
			permissionFlag_ = serviceInfo.getPermissionFlag();
			serviceInfo.getParameterList(parameterList_);
			server_ = serviceInfo.getServer();
            serviceResultFlag_ = serviceInfo.getServiceResultFlag();

			return *this;
		}
        */

		/**
		 * @brief Set server information
		 * @param
		 * serverIP - the IP of the server
		 * @param
		 * serverPort - the port of the server
		 */
		void ServiceInfo::setServer(const std::string& serverIP, unsigned int serverPort)
		{
			server_.nodeIP_ = serverIP;
			server_.nodePort_ = serverPort;
		}

        // ADDED @by MyungHyun (Kent) -2008-11-27
        void ServiceInfo::setServer( const MessageFrameworkNode & server )
        {
            server_ = server;
        }

		/**
		 * @brief Get server information
		 * @return
		 * The information of the server
		 */
		const MessageFrameworkNode& ServiceInfo::getServer() const
		{
			return server_;
		}

		/**
		 * @brief Set permission flag
		 * @param
		 * flagValue - the new permission flag value
		 */
		//void ServiceInfo::setPermissionFlag(PermissionFlag flagValue)
		//{
		//	permissionFlag_ = flagValue;
		//}


		/**
		 * @brief Get permission flag
		 * @return the permission flag value
		 */
		//const PermissionFlag& ServiceInfo::getPermissionFlag() const
		//{
		//	return permissionFlag_;
		//}

		/**
		 * @brief Set service name
		 * @param
		 * serviceName - new service name
		 */
		void ServiceInfo::setServiceName(const std::string& serviceName)
		{
			serviceName_ = serviceName;
		}

		/**
		 * @brief Set service name
		 * @return 
		 *  the service name
		 */
		const std::string& ServiceInfo::getServiceName() const
		{
			return serviceName_;
		}

		/**
		 * @brief set the list of input parameters of the service
		 * @param
		 * parameterList - the new list of parameters
		 */
		//void ServiceInfo::setParameterList(const std::vector<ServiceParameterType>& parameterList)
		//{
		//	parameterList_.clear();
		//	for(size_t i = 0; i < parameterList.size(); i++)
		//		parameterList_.push_back(parameterList[i]);
		//}

		/**
		 * @brief get the list of parameters of the service
		 * @param
		 * parameterList - the list of parameters
		 */
		//void ServiceInfo::getParameterList(std::vector<ServiceParameterType>& parameterList) const
		//{
			//parameterList.clear();
			//for(size_t i = 0; i < parameterList_.size(); i++)
			//	parameterList.push_back(parameterList_[i]);
		//}

		/**
		 * @brief retrieve the key value (i.e., service name) of the service
		 * This function is used by LinearHashTable
		 * @return
		 * key value (i.e., service name) of the service
		 */
		//const std::string& ServiceInfo::get_key() const
		//{
		//	return serviceName_;
		//}

}// end of messageframework

