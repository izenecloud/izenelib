///
///  @file : ServiceInfo.h
///  @date : 8/5/2008
///  @author : TuanQuang Nguyen
///


#if !defined(_SERVICEINFO_H)
#define _SERVICEINFO_H

#include <net/message-framework/MessageFrameworkNode.h>
#include <net/message-framework/ServiceParameterType.h>

#include <string>
#include <vector>


namespace messageframework
{
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

		ServiceInfo(const ServiceInfo& inputService);

		/**
		 * @brief destructor, destroy variables if it is neccessary
		 */
		~ServiceInfo();

		/**
 		 * @brief clear internal data
 		 */
		void clear();

        // @by MyungHyun - 2009-01-28
		//ServiceInfo& operator=(const ServiceInfo& serviceInfo);

		/**
		 * @brief Set server information
		 * @param
		 * serverIP - the IP of the server
		 * @param
		 * serverPort - the port of the server
		 */
		void setServer(const std::string& serverIP, unsigned int serverPort);
        
        // ADDED @by MyungHyun (Kent) -2008-11-27
       void setServer( const MessageFrameworkNode & server );

		/**
		 * @brief Get server information
		 * @return
		 * The information of the server
		 */
		const MessageFrameworkNode& getServer() const;

		/**
		 * @brief Set permission flag
		 * @param
		 * flagValue - the new permission flag value
		 */
		void setPermissionFlag(PermissionFlag flagValue);


		/**
		 * @brief Get permission flag
		 * @return the permission flag value
		 */
		const PermissionFlag& getPermissionFlag() const;

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

		/**
		 * @brief set the list of input parameters of the service
		 * @param
		 * parameterList - the new list of parameters
		 */
		void setParameterList(const std::vector<ServiceParameterType>& parameterList);

		/**
		 * @brief get the list of parameters of the service
		 * @param
		 * parameterList - the list of parameters
		 */
		void getParameterList(std::vector<ServiceParameterType>& parameterList) const;

		/**
		 * @brief       get the list of parameters of the service
		 * @return      parameterList - the list of parameters
         * @by MyungHyun - Feb 10, 2009
		 */
		const std::vector<ServiceParameterType> & getParameterList() const
        {
            return parameterList_;
        }

		/**
		 * @brief retrieve the key value (i.e., service name) of the service
		 * This function is used by LinearHashTable
		 * @return
		 * key value (i.e., service name) of the service
		 */
		const std::string& get_key() const;


        /**
         * @brief   Sets the return flag of this service
         */
        void setServiceResultFlag( const ServiceResultFlag flag )
        {
            serviceResultFlag_ = flag;
        }

        /**
         * @brief   Returns the return flag of this service
         */
        ServiceResultFlag getServiceResultFlag() const
        {
            return serviceResultFlag_;
        }


		template <typename Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & server_ & serviceName_ & permissionFlag_ & parameterList_ & serviceResultFlag_;
		}
     void display(std::ostream &os=std::cout) const {
     	  os<<"ServiceInfo: ";
     	  os<<serviceName_<<std::endl;
     	  os<<server_.nodeName_<<std::endl;
     	  os<<permissionFlag_<<std::endl;
     	  os<<serviceResultFlag_<<std::endl;
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

		/**
		 * @brief The list of parameter types of the service
		 */
		std::vector<ServiceParameterType> parameterList_;

		/**
		 * @brief The permissioni flag tells how to serve the client (directly or indirectly)
		 */
		PermissionFlag permissionFlag_;

        /**
         * @brief   Shows if this service returns a result
         */
      ServiceResultFlag serviceResultFlag_;
	};
  // typedef boost::shared_ptr<ServiceInfo> ServiceInfoPtr;
	
}// end of messageframework
#endif  //_SERVICEINFO_H
