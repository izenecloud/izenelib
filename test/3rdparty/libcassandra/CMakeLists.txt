INCLUDE_DIRECTORIES(
    ${Thrift_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${IZENELIB_SOURCE_DIR}/include/3rdparty/
    ${Glog_INCLUDE_DIRS}
)
ADD_EXECUTABLE(simple simple.cc) 
TARGET_LINK_LIBRARIES(simple cassandra ${Thrift_LIBS}) 
