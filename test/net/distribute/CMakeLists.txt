include_directories(${IZENELIB_SOURCE_DIR}/source)
set(SOURCE_DIR ${IZENELIB_SOURCE_DIR}/source/net/distributed)

find_package(Boost ${Boost_FIND_VERSION}
    REQUIRED COMPONENTS
#    regex
#    system
#    thread
    unit_test_framework
)
add_definitions(-DBOOST_TEST_DYN_LINK)
include_directories(${Boost_INCLUDE_DIRS})
list(APPEND LIBS ${Boost_LIBRARIES})

find_package(Glog)
include_directories(${Glog_INCLUDE_DIRS})
list(APPEND LIBS ${Glog_LIBRARIES})
list(APPEND LIBS pthread)

# TODO: remove when finished
# t_data_receiver
add_executable(t_data_receiver t_data_receiver.cpp)
target_link_libraries(t_data_receiver distribute ${LIBS})

# t_data_transfer
add_executable(t_data_transfer t_data_transfer.cpp)
target_link_libraries(t_data_transfer distribute ${LIBS})

# DataTransfer2
add_executable(t_DataTransfer2 DataTransfer2Test.cpp)
target_link_libraries(t_DataTransfer2 distribute ${LIBS})
