FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS
  system
  thread
  filesystem
  serialization
  unit_test_framework
  program_options
)

INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${IZENELIB_SOURCE_DIR}/include/3rdparty/
)

SET(libs
   zookeeper
   ${Boost_LIBRARIES}
   izene_util
   febird
   ${Glog_LIBRARIES}
)


SET(t_zookeeper_SRC
  Runner.cpp
  t_zookeeper.cpp
)

ADD_EXECUTABLE(t_zookeeper ${t_zookeeper_SRC})
TARGET_LINK_LIBRARIES(t_zookeeper ${libs})

ADD_EXECUTABLE(t_zk_client t_zk_client.cpp)
TARGET_LINK_LIBRARIES(t_zk_client ${libs})

ADD_EXECUTABLE(zkClient zkClient.cpp)
TARGET_LINK_LIBRARIES(zkClient ${libs})
