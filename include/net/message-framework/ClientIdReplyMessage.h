///
/// @file ClientIdReplyMessage.h
/// @date 17/02/2009
/// @author Wei Cao
//


#ifndef _CLIENT_ID_REPLY_MESSAGE_H_
#define _CLIENT_ID_REPLY_MESSAGE_H_

namespace messageframework
{

	/**
	 * @brief This class defines message sent from MessageController to MessageClient
   *    in order to send back client id.
	 */
	class ClientIdReplyMessage
	{
		public:
		/**
		 * @brief Default constructor is must be defined. boost::serialization will use
		 * this constructor
		 */
		ClientIdReplyMessage(){}

		/**
		 * @brief Constructor
		 */
		ClientIdReplyMessage(int clientId) {
		  clientId_ = clientId;
		}

		inline int getClientId() {
		  return clientId_;
		}

		/**
		 * @brief This function is used by boost:serialization
		 */
		template <typename Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & clientId_;
		}

		private:
      int clientId_;

	};

}// end of namespace messageframework

#endif
