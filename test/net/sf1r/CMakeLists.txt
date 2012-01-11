include_directories(${IZENELIB_SOURCE_DIR}/source)
set(SOURCE_DIR ${IZENELIB_SOURCE_DIR}/source/net/sf1r)

find_package(Boost ${Boost_FIND_VERSION} COMPONENTS unit_test_framework system)
add_definitions(-DBOOST_TEST_DYN_LINK)
include_directories(${Boost_INCLUDE_DIRS})
set(LIBS ${LIBS} ${Boost_LIBRARIES})

find_package(Glog)
include_directories(${Glog_INCLUDE_DIRS})
set(LIBS ${LIBS} ${Glog_LIBRARIES})


# Utils
set(Utils_SOURCES
    UtilsTest.cpp   
)
add_executable(t_UtilsTest ${Utils_SOURCES})
target_link_libraries(t_UtilsTest sf1r ${LIBS})


# RawClient
set(RawClient_SOURCES
    ${SOURCE_DIR}/RawClient.cpp
    RawClientTest.cpp
)
add_executable(t_RawClientTest ${RawClient_SOURCES})
target_link_libraries(t_RawClientTest sf1r ${LIBS})


# Rapidjson
set(Rapidjson_SOURCES
    RapidjsonTest.cpp
)
add_executable(t_RapidjsonTest ${Rapidjson_SOURCES})
target_link_libraries(t_RapidjsonTest ${LIBS})


# JsonWriter
set(JsonWriter_SOURCES
    ${SOURCE_DIR}/JsonWriter.cpp
    JsonWriterTest.cpp
)
add_executable(t_JsonWriterTest ${JsonWriter_SOURCES})
target_link_libraries(t_JsonWriterTest ${LIBS})


# Sf1Driver
set(Sf1Driver_SOURCES
    ${SOURCE_DIR}/RawClient.cpp
    ${SOURCE_DIR}/Sf1Driver.cpp
    Sf1DriverTest.cpp
)
add_executable(t_Sf1DriverTest ${Sf1Driver_SOURCES})
target_link_libraries(t_Sf1DriverTest sf1r ${LIBS})
