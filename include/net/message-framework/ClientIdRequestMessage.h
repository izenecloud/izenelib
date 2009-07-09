///
/// @file ClientIdRequestMessage.h
/// @date 17/02/2009
/// @author Wei Cao
//


#ifndef _CLIENT_ID_REQUEST_MESSAGE_H_
#define _CLIENT_ID_REQUEST_MESSAGE_H_

namespace messageframework
{

	/**
	 * @brief This class defines message sent from MessageClient to MessageController
	 *    to request a client id.
	 */
	class ClientIdRequestMessage
	{
		public:
		/**
		 * @brief Default constructor is must be defined. boost::serialization will use
		 * this constructor
		 */
		ClientIdRequestMessage(){}

		/**
		 * @brief This function is used by boost:serialization
		 */
		template <typename Archive>
		void serialize(Archive & ar, const unsigned int version){}

	};

}// end of namespace messageframework

#endif
