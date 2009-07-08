/// 
/// @file ServiceRegistrationReplyMessage.h
/// @date 14/8/2008
/// @author TuanQuang Nguyen
//


#ifndef SERVICE_REGISTRATION_REPLY_MESSAGE_H_
#define SERVICE_REGISTRATION_REPLY_MESSAGE_H_

#include <net/message-framework/ServiceInfo.h>

#include <vector>
#include <string>


namespace messageframework
{
		/**
		 * @brief ServiceRegistrationReplyMessage is sent from MessageController to 
		 * MessageServer to notify that a service has been successfully registered.
		 */
		class ServiceRegistrationReplyMessage
		{
			public:
				ServiceRegistrationReplyMessage():status_(false){}

				ServiceRegistrationReplyMessage(const std::string serviceName, bool status)
					:status_(status),
					serviceName_(serviceName)
				{
				}

				const std::string& getServiceName(){return serviceName_;}
				bool getStatus(){return status_;}

				template <typename Archive>
				void serialize(Archive & ar, const unsigned int version)
				{
					ar & status_ & serviceName_;
				}
			private:
				bool status_;
				std::string serviceName_;
		};

	// typedef boost::shared_ptr<ServiceRegistrationReplyMessage> ServiceRegistrationReplyMessagePtr;
}// end of namespace messageframework

#endif // end of #ifndef SERVICE_REGISTRATION_REPLY_MESSAGE_H_
