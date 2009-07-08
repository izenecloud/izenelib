///
///  @file : ClientIdRequester.h
///  @date : 17/02/2009
///  @author : Wei Cao
///  @brief This file define ClientIdRequester abstract class
///



#ifndef _CLIENT_ID_REQUESTER_H_
#define _CLIENT_ID_REQUESTER_H_

namespace messageframework
{
	/**
   * @brief This class defines interfaces of object that requests client id service
   */
	class ClientIdRequester
	{
	public:
		ClientIdRequester(){}
		virtual ~ClientIdRequester(){}

    /**
     * @brief This function sends a request to get client id
     */
		virtual void sendClientIdRequest() {}

    /**
     * @brief This function saves client id
     * @param
     * clientId - controller assigned client id
     */
		virtual void receiveClientIdResult(const int& clientId) {}
	};
}// end of namespace messageframework

#endif
