INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

SET_PROPERTY(GLOBAL APPEND PROPERTY BUILD_COMPONENTS messageframework)

SET(messageframework_SRC
  AsyncConnector.cpp
  AsyncStream.cpp
  MessageAgent.cpp
  MessageClientFull.cpp
  MessageControllerFull.cpp
  MessageDispatcher.cpp
  MessageFrameworkException.cpp
  MessageFrameworkNode.cpp
  MessageServerFull.cpp
  ServiceInfo.cpp
  ServicePermissionInfo.cpp
  ServiceRegistrationMessage.cpp
  ThreadPool.cpp
  common.cpp     
  )

MACRO_ADD_COMPONENT(messageframework
  "${VERSION_INSTALL_PREFIX}"
  "${PROJECT_VERSION}"
  ${messageframework_SRC}
  )
