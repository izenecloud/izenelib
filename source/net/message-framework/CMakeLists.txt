INCLUDE_DIRECTORIES(
  ${Boost_INCLUDE_DIRS}  
  ${wiselib_INCLUDE_DIRS}
  ${Glog_INCLUDE_DIRS}
)

IF(WIN32)
  SET_PROPERTY(GLOBAL APPEND PROPERTY NON_BUILD_COMPONENTS message_framework)
  MESSAGE(STATUS "! WARNING: message_framework can only been build in WIN32")

ELSE(WIN32)

  SET_PROPERTY(GLOBAL APPEND PROPERTY BUILD_COMPONENTS message_framework)

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
    common.cpp   
    
    MFClient.cpp  
    
  #  MessageClientLight.cpp
  #  MessageControllerLight.cpp
  #  MessageServerLight.cpp
  #  MessageAgentLight.cpp
)

  INCLUDE(MacroAddComponent)
  MACRO_ADD_COMPONENT(message_framework
    "${VERSION_INSTALL_PREFIX}"
    "${PROJECT_VERSION}"
    ${message_framework_SRC}
    )

ENDIF(WIN32)

