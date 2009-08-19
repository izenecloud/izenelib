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
		}
		
		ServiceInfo::ServiceInfo(const std::string& serviceName)
		{
					server_.nodeIP_ = "";
					server_.nodePort_ = 0;
					serviceName_ = serviceName;
		}

		ServiceInfo::ServiceInfo(const ServiceInfo& inputService)
		{
			server_ = inputService.getServer();	
			serviceName_ = inputService.getServiceName();		
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
		}

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

}// end of messageframework

