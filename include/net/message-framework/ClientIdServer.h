///
///  @file : ClientIdServer.h
///  @date : 17/02/2009
///  @author : Wei Cao
///  @brief This file define ClientIdServer abstract class
///


#ifndef _CLIENT_ID_SERVER_H_
#define _CLIENT_ID_SERVER_H_

#include <net/message-framework/MessageFrameworkNode.h>

namespace messageframework
{
	/**
   * @brief This class defines interfaces of object that serves requests of
   *    client id service
   */
	class ClientIdServer
	{
	public:
		ClientIdServer(){}
		virtual ~ClientIdServer(){}

		/**
 		 * @brief When a new request of get client id comes, this function
 		 *    is called in order to notify the server
 	 	 * @param requester - node that requests a client id
 		 */
		virtual void receiveClientIdRequest(const MessageFrameworkNode& requester){}

		/**
 	 	 * @brief This function replies to a request from ClientIdRequester
 	 	 * @param requester - node that requests for a permission
 	 	 * @param clientId - assigned client id
 	 	 */
		virtual void sendClientIdResult(const MessageFrameworkNode& requester,
							const int& clientId) {}
	};
}// end of namespace messageframework

#endif
