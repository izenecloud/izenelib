///
/// @file PermissionOfServiceMessage.h
/// @date 14/8/2008
/// @author TuanQuang Nguyen
//


#ifndef _PERMISSION_OF_SERVICE_MESSAGE_H_
#define _PERMISSION_OF_SERVICE_MESSAGE_H_

#include <net/message-framework/ServicePermissionInfo.h>


namespace messageframework
{
		/**
		 * @brief This class defines message between MessageClient and MessageController
		 * when MessageClient requests ServicePermissionInfo.
		 */
		class PermissionOfServiceMessage
		{
				public:
						/**
						 * @brief Default constructor must be defined. boost::serialization will use
						 * this constructor
						 */
						PermissionOfServiceMessage()
						{
								servicePermissionInfo_.clear();
						}
						PermissionOfServiceMessage(const ServicePermissionInfo& servicePermissionInfo):
							servicePermissionInfo_(servicePermissionInfo)
						{
						}

						/**
						 * @brief Construct a mesage with a given service name
						 * @param
						 * serviceName - the service name
						 */
						PermissionOfServiceMessage(const std::string& serviceName)
						{
								servicePermissionInfo_.clear();
								servicePermissionInfo_.setServiceName(serviceName);
						}

						/**
						 * @brief Set the permission flag of the service
						 * @param
						 * permissionFlag - the permission flag
						 */
						void setPermissionFlag(PermissionFlag permissionFlag)
						{
								servicePermissionInfo_.setPermissionFlag(permissionFlag);
						}

						/**
						 * @brief Get the permission flag of the service
						 * @return
						 * the permission flag
						 */
						const PermissionFlag& getPermissionFlag(PermissionFlag permissionFlag)
						{
								return servicePermissionInfo_.getPermissionFlag();
						}

						/**
						 * @brief Set the service name
						 * @param
						 * serviceName - the service name
						 */
						void setServiceName(const std::string& serviceName)
						{
								servicePermissionInfo_.setServiceName(serviceName);
						}

						/**
						 * @brief Get the service name
						 * @return
						 * The service name
						 */
						const std::string& getServiceName()
						{
								return servicePermissionInfo_.getServiceName();
						}

						/**
						 * @brief set the server of the service
						 * @param
						 * ipAddress - IPAddress of the server
						 * @param
						 * port - port of the server
						 */
						void setServer(const std::string& ipAddress, unsigned int port)
						{
								servicePermissionInfo_.setServer(ipAddress, port);
						}

						/**
						 * @brief get the server of the service
						 * @return
						 * ipAddress and port of the server
						 */
						const MessageFrameworkNode& getServer()
						{
								return servicePermissionInfo_.getServer();
						}

						/**
						 * @brief get service permission information
						 * @return
						 * reference to ServicePermissionInfo
						 */
						const ServicePermissionInfo& getServicePermissionInfo()
						{
							return servicePermissionInfo_;
						}


						/**
						 * @brief This function is used by boost:serialization
						 */
						template <typename Archive>
								void serialize(Archive & ar, const unsigned int version)
								{
										ar & servicePermissionInfo_;
								}
				protected:
						/**
						 * @brief information of the service permission.
						 */
						ServicePermissionInfo servicePermissionInfo_;


		};

		class PermissionRequestMessage
		{
			public:
				PermissionRequestMessage(){}

				PermissionRequestMessage(const std::string& serviceName):
						serviceName_(serviceName)
				{
				}

				~PermissionRequestMessage(){}
				const std::string getServiceName(){ return serviceName_;}

				/**
				 * @brief This function is used by boost:serialization
				 */
				template <typename Archive>
				void serialize(Archive & ar, const unsigned int version)
				{
					ar & serviceName_;
				}

			private:
				std::string serviceName_;
		};

}// end of namespace messageframework

#endif
