INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${IZENELIB_SOURCE_DIR}/include/3rdparty/
    ${IZENELIB_SOURCE_DIR}/include/3rdparty/libcassandra/genthrift
    ${Thrift_INCLUDE_DIR}
    ${Glog_INCLUDE_DIRS}
)

# third party libraries
# put it first, they may be referred in other components
FILE(GLOB_RECURSE libcassandra_SRC 
  "genthrift/*.cpp"
  "*.cc" )
FILE(GLOB cassandra_server_SRC RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "genthrift/Cassandra_server.skeleton.cpp")   
LIST(REMOVE_ITEM libcassandra_SRC ${cassandra_server_SRC}) 
ADD_DEFINITIONS("-fPIC")
ADD_LIBRARY(cassandra ${libcassandra_SRC} ) 
TARGET_LINK_LIBRARIES(cassandra ${Thrift_LIBS} ) 
