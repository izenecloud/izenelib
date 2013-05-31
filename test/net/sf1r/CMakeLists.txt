include_directories(${IZENELIB_SOURCE_DIR}/source)
set(SOURCE_DIR ${IZENELIB_SOURCE_DIR}/source/net/sf1r)

find_package(Boost ${Boost_FIND_VERSION}
    REQUIRED COMPONENTS
    regex
    system
    thread
    serialization
    unit_test_framework
)

add_definitions(-DBOOST_TEST_DYN_LINK)
include_directories(${Boost_INCLUDE_DIRS})
list(APPEND LIBS ${Boost_LIBRARIES})

find_package(Glog)
include_directories(${Glog_INCLUDE_DIRS})
list(APPEND LIBS ${Glog_LIBRARIES})
list(APPEND LIBS pthread rt)

option(ENABLE_SF1_TEST "Enable compilation of SF1 tests" false)
if (ENABLE_SF1_TEST)
    message(STATUS "enabling SF1 tests")
    add_definitions(-DENABLE_SF1_TEST)
endif()

option(ENABLE_ZK_TEST "Enable compilation of ZooKeeper tests" false)
if (ENABLE_ZK_TEST)
    message(STATUS "enabling ZooKeeper and SF1 tests")
    add_definitions(-DENABLE_ZK_TEST)
endif()


# Utils
set(Utils_SOURCES 
    UtilsTest.cpp
)
add_executable(t_Utils ${Utils_SOURCES})
target_link_libraries(t_Utils ${LIBS})
list(APPEND ALL_TESTS t_Utils)


# RawClient
set(RawClient_SOURCES
    ${SOURCE_DIR}/RawClient.cpp
    RawClientTest.cpp
)
add_executable(t_RawClient ${RawClient_SOURCES})
target_link_libraries(t_RawClient ${LIBS} ${CMAKE_THREAD_LIBS_INIT})
list(APPEND ALL_TESTS t_RawClient)


# Rapidjson
set(Rapidjson_SOURCES
    RapidjsonTest.cpp
)
add_executable(t_Rapidjson ${Rapidjson_SOURCES})
target_link_libraries(t_Rapidjson ${LIBS})
list(APPEND ALL_TESTS t_Rapidjson)


# JsonWriter
set(JsonWriter_SOURCES
    ${SOURCE_DIR}/JsonWriter.cpp
    JsonWriterTest.cpp
)
add_executable(t_JsonWriter ${JsonWriter_SOURCES})
target_link_libraries(t_JsonWriter ${LIBS} ${CMAKE_THREAD_LIBS_INIT})
list(APPEND ALL_TESTS t_JsonWriter)


# RegexLexer
set(RegexLexer_SOURCES
    distributed/RegexLexerTest.cpp
)
add_executable(t_RegexLexer ${RegexLexer_SOURCES})
target_link_libraries(t_RegexLexer ${LIBS})
list(APPEND ALL_TEST t_RegexLexer)


# ConnectionPool
set(ConnectionPool_SOURCES
    ${SOURCE_DIR}/RawClient.cpp
    ${SOURCE_DIR}/ConnectionPool.cpp
    ConnectionPoolTest.cpp
)
add_executable(t_ConnectionPool ${ConnectionPool_SOURCES})
target_link_libraries(t_ConnectionPool ${LIBS} ${CMAKE_THREAD_LIBS_INIT})
list(APPEND ALL_TESTS t_ConnectionPool)


# PoolFactory
set(PoolFactory_SOURCES
    ${SOURCE_DIR}/RawClient.cpp
    ${SOURCE_DIR}/ConnectionPool.cpp
    ${SOURCE_DIR}/distributed/Sf1Node.cpp
    ${IZENELIB_SOURCE_DIR}/source/util/kv2string.cpp
    PoolFactoryTest.cpp
)
add_executable(t_PoolFactory ${PoolFactory_SOURCES})
target_link_libraries(t_PoolFactory ${LIBS})
list(APPEND ALL_TESTS t_PoolFactory)


# Sf1Driver
set(Sf1Driver_SOURCES
    Sf1DriverTest.cpp
)
add_executable(t_Sf1Driver ${Sf1Driver_SOURCES})
target_link_libraries(t_Sf1Driver sf1r ${LIBS} ${CMAKE_THREAD_LIBS_INIT})
list(APPEND ALL_TESTS t_Sf1Driver)


# Sf1Topology
set(Sf1Topology_SOURCES
    ${IZENELIB_SOURCE_DIR}/source/util/kv2string.cpp
    ${SOURCE_DIR}/distributed/Sf1Node.cpp
    ${SOURCE_DIR}/distributed/Sf1Topology.cpp
    distributed/NodeContainerTest.cpp
    distributed/Sf1TopologyTest.cpp)
add_executable(t_Sf1Topology ${Sf1Topology_SOURCES})
target_link_libraries(t_Sf1Topology ${LIBS})
list(APPEND ALL_TESTS t_Sf1Topology)


# RoundRobinPolicy
set(RoundRobinPolicy_SOURCES
    ${IZENELIB_SOURCE_DIR}/source/util/kv2string.cpp
    ${SOURCE_DIR}/distributed/Sf1Node.cpp
    ${SOURCE_DIR}/distributed/Sf1Topology.cpp
    ${SOURCE_DIR}/distributed/RoundRobinPolicy.cpp
    distributed/RoundRobinPolicyTest.cpp
)
add_executable(t_RoundRobinPolicy ${RoundRobinPolicy_SOURCES})
target_link_libraries(t_RoundRobinPolicy ${LIBS})
list(APPEND ALL_TESTS t_RoundRobinPolicy)


# ZooKeeperRouter
set(ZooKeeperRouter_SOURCES
    ${IZENELIB_SOURCE_DIR}/source/util/kv2string.cpp
    ${IZENELIB_SOURCE_DIR}/source/util/scheduler.cpp
    ${IZENELIB_SOURCE_DIR}/source/util/timer.cpp
    ${IZENELIB_SOURCE_DIR}/source/util/singleton.cpp
    ${SOURCE_DIR}/RawClient.cpp
    ${SOURCE_DIR}/ConnectionPool.cpp
    ${SOURCE_DIR}/distributed/Sf1Node.cpp
    ${SOURCE_DIR}/distributed/Sf1Topology.cpp
    ${SOURCE_DIR}/distributed/RoundRobinPolicy.cpp
    ${SOURCE_DIR}/distributed/Sf1Watcher.cpp
    ${SOURCE_DIR}/distributed/ZooKeeperRouter.cpp
    distributed/ZooKeeperRouterTest.cpp
)
add_executable(t_ZooKeeperRouter ${ZooKeeperRouter_SOURCES})
target_link_libraries(t_ZooKeeperRouter zookeeper ${LIBS} ${CMAKE_THREAD_LIBS_INIT})
list(APPEND ALL_TESTS t_ZooKeeperRouter)


# Sf1DistributedDriver
set(Sf1DistributedDriver_SOURCES
    distributed/Sf1DistributedDriverTest.cpp
)
add_executable(t_Sf1DistributedDriver ${Sf1DistributedDriver_SOURCES})
target_link_libraries(t_Sf1DistributedDriver sf1r ${LIBS} ${CMAKE_THREAD_LIBS_INIT})
list(APPEND ALL_TESTS t_Sf1DistributedDriver)


# Sf1Demo
add_executable(Sf1Demo EXCLUDE_FROM_ALL distributed/Sf1Demo.cpp)
target_link_libraries(Sf1Demo sf1r ${Glog_LIBRARIES} ${Boost_LIBRARIES})


# shortcut for building all tests
add_custom_target(Sf1Tests DEPENDS ${ALL_TESTS})
