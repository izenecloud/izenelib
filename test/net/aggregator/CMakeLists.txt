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
   aggregator
   msgpack
   febird
   izene_util
   ${Boost_LIBRARIES}
   ${Glog_LIBRARIES}
)

ADD_EXECUTABLE(t_worker t_worker.cpp)
TARGET_LINK_LIBRARIES(t_worker ${libs})

SET(t_agg_SRC
  Runner.cpp
  t_aggregator.cpp
)

ADD_EXECUTABLE(t_aggregator ${t_agg_SRC})
TARGET_LINK_LIBRARIES(t_aggregator ${libs})

SET(t_CallProxy_SRC
  Runner.cpp
  t_CallProxy.cpp
)

ADD_EXECUTABLE(t_CallProxy ${t_CallProxy_SRC})
TARGET_LINK_LIBRARIES(t_CallProxy ${libs})
