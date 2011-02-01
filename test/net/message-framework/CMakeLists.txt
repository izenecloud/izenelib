FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS
  system
  thread
  serialization
  unit_test_framework
  )
LIST(FIND REAL_BUILD_COMPONENTS "procmeminfo" _procmeminfo)
LIST(FIND REAL_BUILD_COMPONENTS "message_framework" _message_framework)

IF(Boost_FOUND AND Glog_FOUND AND
    _procmeminfo GREATER -1 AND _message_framework GREATER -1)

INCLUDE_DIRECTORIES(
  ${Boost_INCLUDE_DIRS}
  ${Glog_INCLUDE_DIRS}
)

SET(t_mf_SRC
  t_ServiceResolver.cpp
  t_master_suite.cpp
)

ADD_EXECUTABLE(t_mf ${t_mf_SRC})

TARGET_LINK_LIBRARIES(t_mf
  message_framework
  procmeminfo
  ${Boost_LIBRARIES}
  ${Glog_LIBRARIES}
)

ENDIF(Boost_FOUND AND Glog_FOUND AND
  _procmeminfo GREATER -1 AND _message_framework GREATER -1)
