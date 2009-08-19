/**
 * @file    message_framework.h
 * @brief   This header file defines the different settings of Message Framework (full & light). Including this header file 
 *          with the appropriate definition will allow one to use either version of Message Framework with little change in the code.
 * @author  MyungHyun Lee(Kent) and Nguyen, Tuan-Quang
 * @date    2008-11-27
 * @details
 *  - Log
 *      - 2009.08.14 Commented MessageAgent.h by Dohyun Yun.
 */

#ifndef _MESSAGE_FRAMEWORK_H_
#define _MESSAGE_FRAMEWORK_H_

#include <util/izene_log.h>

//---------------  IF USING MESSAGE FRAMEWORK FULL  -----------------
#ifdef USE_MF_LIGHT

#include <net/message-framework/MessageClientLight.h>
#include <net/message-framework/MessageServerLight.h>
#include <net/message-framework/MessageControllerLight.h>
#include <net/message-framework/MFSerialization.h>
#include <net/message-framework/MessageAgentLight.h> 

namespace messageframework
{
	typedef MessageClientLight MessageClient;
	typedef MessageServerLight MessageServer;
	typedef MessageControllerLight MessageController;
	typedef boost::shared_ptr<MessageClient> MessageClientPtr;
	typedef boost::shared_ptr<MessageServer> MessageServerPtr;
	typedef boost::shared_ptr<MessageController> MessageControllerPtr;
}
#define MF_AUTO_MAIN( method_name )         \
    int method_name( int argc, char * argv[])

#define MF_CLIENT_ARG( clientName, controllerInfo )                      \
    clientName, messageframework::MessageController::instance()

#define MF_SERVER_ARG( serverName, serverPort, controllerInfo )          \
    serverName, messageframework::MessageController::instance() 

#define INIT_MF_CONTROLLER( controllerName, controllerPort )

#define GET_CONTROLLER_INSTANCE()           \
    messageframework::MessageController::instance();

//---------------  IF USING MESSAGE FRAMEWORK FULL  -----------------
#else  
#include <net/message-framework/MessageClientFull.h>
#include <net/message-framework/MessageServerFull.h>
#include <net/message-framework/MessageControllerFull.h>
#include <net/message-framework/MessageFrameworkConfiguration.h>
#include <net/message-framework/MessageAgentFull.h> 

namespace messageframework {

typedef MessageClientFull MessageClient;
typedef MessageServerFull MessageServer;
typedef MessageControllerFull MessageController;
typedef boost::shared_ptr<MessageClient> MessageClientPtr;
typedef boost::shared_ptr<MessageServer> MessageServerPtr;
typedef boost::shared_ptr<MessageController> MessageControllerPtr;
}

#define MF_AUTO_MAIN( method_name )                                         \
    int method_name( int argc, char * argv[]);                                  \
    int main( int argc, char * argv[] )                                         \
    {                                                                           \
        method_name( argc, argv );                                              \
    }                                                                           \
    int method_name( int argc, char * argv[])

#define MF_CLIENT_ARG( clientName, controllerInfo )                         \
    clientName, controllerInfo 

#define MF_SERVER_ARG( serverName, serverPort, controllerInfo )             \
    serverName, serverPort, controllerInfo 

#define INIT_MF_CONTROLLER( controllerName, controllerPort )                \
    messageframework::MessageControllerFullSingleton::init( controllerName, controllerPort);

#define GET_CONTROLLER_INSTANCE()           \
    messageframework::MessageControllerFullSingleton::instance();

#endif

#endif  //end of file

