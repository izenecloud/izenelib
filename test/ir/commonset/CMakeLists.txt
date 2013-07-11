FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS
  system
  thread
  filesystem
  serialization
  random
  unit_test_framework
  )

SET(libs
  ${Glog_LIBRARIES}
  ${Boost_LIBRARIES}
  izene_util
  febird
  am
)

IF(Boost_FOUND)

INCLUDE_DIRECTORIES(
  ${Boost_INCLUDE_DIRS}
  ${Glog_INCLUDE_DIRS}
 )

SET(t_commonset_SRC
  t_commonset.cpp
  t_master_suite.cpp
)

ADD_EXECUTABLE(t_commonset ${t_commonset_SRC})

TARGET_LINK_LIBRARIES(t_commonset ${libs})

ENDIF(Boost_FOUND)
