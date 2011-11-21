FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS
  system
  thread
  filesystem
  unit_test_framework
)

INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${IZENELIB_SOURCE_DIR}/include/
)

SET(libs
   distribute
   ${Boost_LIBRARIES}
   ${Glog_LIBRARIES}
)

SET(t_data_transfer_SRC
  t_data_transfer.cpp
)
ADD_EXECUTABLE(t_data_transfer ${t_data_transfer_SRC})
TARGET_LINK_LIBRARIES(t_data_transfer ${libs})

ADD_EXECUTABLE(t_data_receiver t_data_receiver.cpp)
TARGET_LINK_LIBRARIES(t_data_receiver ${libs})

