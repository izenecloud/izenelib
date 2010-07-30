SET(_NOT_WIN32 TRUE)
IF(WIN32)
  SET(_NOT_WIN32 FALSE)
ENDIF(WIN32)

MACRO_CHECK_SHOULD_BUILD(
  message_framework
  _NOT_WIN32
  Boost_FOUND
  Glog_FOUND
  )

IF(message_framework_SHOULD_BUILD)

  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${Glog_INCLUDE_DIRS}
    )

  SET(message_framework_SRC

    AsyncConnector.cpp
    AsyncStream.cpp
    MessageAgentFull.cpp
    MessageClientFull.cpp
    MessageControllerFull.cpp
    MessageServerFull.cpp
    MessageDispatcher.cpp

    MessageFrameworkException.cpp
    MessageFrameworkNode.cpp
    ServiceInfo.cpp
    ServicePermissionInfo.cpp
    ServiceRegistrationMessage.cpp
    ThreadPool.cpp
    #MFClient.cpp

    MessageServerLight.cpp
    MessageClientLight.cpp
    MessageControllerLight.cpp
    MessageAgentLight.cpp
    )

  SET(message_framework_light_SRC

    MessageFrameworkException.cpp
    MessageFrameworkNode.cpp
    ServiceInfo.cpp
    ServicePermissionInfo.cpp
    ServiceRegistrationMessage.cpp
    ThreadPool.cpp
    #MFClient.cpp

    MessageServerLight.cpp
    MessageClientLight.cpp
    MessageControllerLight.cpp
    MessageAgentLight.cpp
    )

  MACRO_ADD_LIBRARY(message_framework SRCS ${message_framework_SRC})
  MACRO_ADD_LIBRARY(message_framework_light message_framework SRCS ${message_framework_light_SRC})

ENDIF(message_framework_SHOULD_BUILD)
