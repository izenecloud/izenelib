FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS
  thread
  system
  unit_test_framework
  program_options
  )

INCLUDE_DIRECTORIES(
    ${Thrift_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${IZENELIB_SOURCE_DIR}/include/3rdparty/
    ${Glog_INCLUDE_DIRS}
)

ADD_EXECUTABLE(t_libcassandra t_libcassandra.cc)
TARGET_LINK_LIBRARIES(t_libcassandra 
    cassandra
    febird
    izene_util
    ${Thrift_LIBS} 
    ${Boost_LIBRARIES}
    ${Glog_LIBRARIES}
)
