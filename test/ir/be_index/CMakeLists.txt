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
  json
)

IF(Boost_FOUND)

INCLUDE_DIRECTORIES(
  ${Boost_INCLUDE_DIRS}
  ${Glog_INCLUDE_DIRS}
 )

SET(t_be_index_SRC
  t_be_index.cpp
  t_master_suite.cpp
)

ADD_EXECUTABLE(t_be_index ${t_be_index_SRC})

TARGET_LINK_LIBRARIES(t_be_index ${libs})

ENDIF(Boost_FOUND)
