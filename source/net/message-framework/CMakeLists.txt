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
    
  IF(NOT USE_MF_LIGHT)  

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
    
  ELSE(NOT USE_MF_LIGHT)    

   SET(message_framework_SRC 
  
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

  ENDIF(NOT USE_MF_LIGHT)    

  ADD_LIBRARY(message_framework ${message_framework_SRC})
  #INSTALL(TARGETS message_framework DESTINATION lib)
  INSTALL(TARGETS message_framework   
      LIBRARY DESTINATION lib 
      ARCHIVE DESTINATION lib 
      COMPONENT "izenelib_libraries")

ENDIF(message_framework_SHOULD_BUILD)
