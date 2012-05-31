include_directories(${IZENELIB_SOURCE_DIR}/source)
set(SOURCE_DIR ${IZENELIB_SOURCE_DIR}/source/net/distributed)

find_package(Boost ${Boost_FIND_VERSION}
    REQUIRED COMPONENTS
    system
    unit_test_framework
)
add_definitions(-DBOOST_TEST_DYN_LINK)
include_directories(${Boost_INCLUDE_DIRS})
list(APPEND LIBS ${Boost_LIBRARIES})

find_package(Glog)
include_directories(${Glog_INCLUDE_DIRS})
list(APPEND LIBS ${Glog_LIBRARIES})
list(APPEND LIBS pthread)

option(ENABLE_FT_TEST "Enable tests for izelib::net::distribute" false)
if(ENABLE_FT_TEST)
  add_definitions(-DENABLE_TEST)
endif()


# DataTransfer2
add_executable(t_DataTransfer2 DataTransfer2Test.cpp)
target_link_libraries(t_DataTransfer2 distribute ${LIBS})

# DataReceiver2
add_executable(t_DataReceiver2 DataReceiver2Test.cpp)
target_link_libraries(t_DataReceiver2 distribute ${LIBS})

# Message
add_executable(t_Message MessageTest.cpp)
target_link_libraries(t_Message distribute ${LIBS})

# ScpHelper
add_executable(t_ScpHelper ScpHelperTest.cpp)
target_link_libraries(t_ScpHelper ${LIBS})

# shortcut for building all tests
add_custom_target(distribute_test DEPENDS 
    t_DataTransfer2 
    t_DataReceiver2 
    t_Message
    t_ScpHelper
)
