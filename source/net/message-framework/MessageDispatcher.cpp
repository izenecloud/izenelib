///
///  @file : MessageDispatcher.cpp
///  @date : 20/11/2008
///  @author : TuanQuang Nguyen
///  @brief This file implements MessageDispatcher that reponsible for sending
///  data using streams
///

#include <net/message-framework/MessageDispatcher.h>
#include <net/message-framework/MessageType.h>
#include <net/message-framework/ServiceRegistrationMessage.h>
#include <net/message-framework/ServiceRegistrationReplyMessage.h>

#include <net/message-framework/ServicePermissionInfo.h>
#include <net/message-framework/ServiceMessage.h>


#include <net/message-framework/MessageClientFull.h>
#include <net/message-framework/MessageControllerFull.h>
#include <net/message-framework/MessageServerFull.h>

#include <boost/archive/archive_exception.hpp>
#include <boost/exception.hpp>
#include <boost/exception/diagnostic_information.hpp>

using namespace std;

namespace messageframework {
    void sendDataToUpperLayer_impl(  MessageServerFull& server, const MessageFrameworkNode& source,
            MessageType messageType, boost::shared_ptr<izenelib::util::izene_streambuf>& buffer)
    {
        switch (messageType)
        {
        case SERVICE_REGISTRATION_REPLY_MSG:
        {
            ServiceRegistrationReplyMessage msg;
            from_buffer(msg, buffer);
            server.receiveServiceRegistrationReply(
                msg.getServiceName(), msg.getStatus());
            break;
        }

        case SERVICE_REQUEST_MSG:
        {
            ServiceMessagePtr msg(new ServiceMessage);
            from_buffer(*msg, buffer);
            server.receiveServiceRequest(source, msg);
            break;
        }

        default:
            throw MessageFrameworkException(SF1_MSGFRK_UNKNOWN_DATATYPE, __LINE__, __FILE__);
        }
    }

    void sendDataToUpperLayer_impl( MessageControllerFull& controller, const MessageFrameworkNode& source,
            MessageType messageType, boost::shared_ptr<izenelib::util::izene_streambuf>& buffer)
    {
        switch (messageType)
        {
        case SERVICE_REGISTRATION_REQUEST_MSG:
        {
            ServiceRegistrationMessage msg;
            from_buffer(msg, buffer);
            controller.receiveServiceRegistrationRequest(source,msg);
            break;
        }

        case PERMISSION_OF_SERVICE_REQUEST_MSG:
        {
            std::string msg;
            from_buffer(msg, buffer);
            controller.receivePermissionOfServiceRequest(source, msg);
            break;
        }

        default:
            throw MessageFrameworkException(SF1_MSGFRK_UNKNOWN_DATATYPE, __LINE__, __FILE__);
        }
    }

    void sendDataToUpperLayer_impl( MessageClientFull& client, const MessageFrameworkNode& source,
            MessageType messageType, boost::shared_ptr<izenelib::util::izene_streambuf>& buffer)
    {
        switch (messageType)
        {

        case SERVICE_RESULT_MSG:
        {
            ServiceMessagePtr msg(new ServiceMessage);
            from_buffer(*msg, buffer);
            client.receiveResultOfService(msg);
            break;
        }

        case PERMISSION_OF_SERVICE_REPLY_MSG:
        {
            ServicePermissionInfo msg;
            from_buffer(msg, buffer);
            client.receivePermissionOfServiceResult(msg);
            break;
        }

        default:
            throw MessageFrameworkException(SF1_MSGFRK_UNKNOWN_DATATYPE, __LINE__, __FILE__);
        }
    }

}// end of namespace messageframework

