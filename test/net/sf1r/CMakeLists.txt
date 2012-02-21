include_directories(${IZENELIB_SOURCE_DIR}/source)
set(SOURCE_DIR ${IZENELIB_SOURCE_DIR}/source/net/sf1r)

find_package(Boost ${Boost_FIND_VERSION}
    REQUIRED COMPONENTS
    regex
    system
    thread
    unit_test_framework
)

add_definitions(-DBOOST_TEST_DYN_LINK)
include_directories(${Boost_INCLUDE_DIRS})
list(APPEND LIBS ${Boost_LIBRARIES})

find_package(Glog)
include_directories(${Glog_INCLUDE_DIRS})
list(APPEND LIBS ${Glog_LIBRARIES})

option(ENABLE_SF1_TEST "Enable compilation of SF1 tests" false)
if (ENABLE_SF1_TEST)
    message(STATUS "enabling SF1 tests")
    add_definitions(-DENABLE_SF1_TEST)
endif()

option(ENABLE_ZK_TEST "Enable compilation of ZooKeeper and SF1 tests" false)
if (ENABLE_ZK_TEST)
    message(STATUS "enabling ZooKeeper and SF1 tests")
    add_definitions(-DENABLE_ZK_TEST)
endif()


# Utils
set(Utils_SOURCES UtilsTest.cpp)
add_executable(t_UtilsTest ${Utils_SOURCES})
target_link_libraries(t_UtilsTest ${LIBS})
list(APPEND ALL_TESTS t_UtilsTest)


# RawClient
set(RawClient_SOURCES
    ${SOURCE_DIR}/RawClient.cpp
    RawClientTest.cpp
)
add_executable(t_RawClientTest ${RawClient_SOURCES})
target_link_libraries(t_RawClientTest ${LIBS} ${CMAKE_THREAD_LIBS_INIT})
list(APPEND ALL_TESTS t_RawClientTest)


# Rapidjson
set(Rapidjson_SOURCES
    RapidjsonTest.cpp
)
add_executable(t_RapidjsonTest ${Rapidjson_SOURCES})
target_link_libraries(t_RapidjsonTest ${LIBS})
list(APPEND ALL_TESTS t_RapidjsonTest)


# JsonWriter
set(JsonWriter_SOURCES
    ${SOURCE_DIR}/JsonWriter.cpp
    JsonWriterTest.cpp
)
add_executable(t_JsonWriterTest ${JsonWriter_SOURCES})
target_link_libraries(t_JsonWriterTest ${LIBS} ${CMAKE_THREAD_LIBS_INIT})
list(APPEND ALL_TESTS t_JsonWriterTest)


# ConnectionPool
set(ConnectionPool_SOURCES
    ${SOURCE_DIR}/RawClient.cpp
    ${SOURCE_DIR}/ConnectionPool.cpp
    ConnectionPoolTest.cpp
)
add_executable(t_ConnectionPoolTest ${ConnectionPool_SOURCES})
target_link_libraries(t_ConnectionPoolTest ${LIBS} ${CMAKE_THREAD_LIBS_INIT})
list(APPEND ALL_TESTS t_ConnectionPoolTest)


# Sf1Driver
set(Sf1Driver_SOURCES
    ${SOURCE_DIR}/RawClient.cpp
    ${SOURCE_DIR}/Sf1Driver.cpp
    Sf1DriverTest.cpp
)
add_executable(t_Sf1DriverTest ${Sf1Driver_SOURCES})
target_link_libraries(t_Sf1DriverTest sf1r ${LIBS} ${CMAKE_THREAD_LIBS_INIT})
list(APPEND ALL_TESTS t_Sf1DriverTest)


# ZooKeeperRouter
set(ZooKeeperRouter_SOURCES
    ${SOURCE_DIR}/Sf1Node.cpp
    ${SOURCE_DIR}/Sf1Watcher.cpp
    ${SOURCE_DIR}/ZooKeeperRouter.cpp
    ${IZENELIB_SOURCE_DIR}/source/util/kv2string.cpp
    ZooKeeperRouterTest.cpp
)
add_executable(t_ZooKeeperRouterTest ${ZooKeeperRouter_SOURCES})
target_link_libraries(t_ZooKeeperRouterTest sf1r zookeeper ${LIBS} ${CMAKE_THREAD_LIBS_INIT})
list(APPEND ALL_TESTS t_ZooKeeperRouterTest)

# shortcut for building all tests
add_custom_target(Sf1Tests DEPENDS ${ALL_TESTS})
