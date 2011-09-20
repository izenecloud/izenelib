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
    ${IZENELIB_SOURCE_DIR}
    ${IZENELIB_SOURCE_DIR}/source/
)

SET(libs
   izene_util
   mrlite
   ${Boost_LIBRARIES}
   ${Glog_LIBRARIES}
)

SET(t_communicator_SRC
  Runner.cpp
  t_communicator.cpp
  wordcount.cpp
)

ADD_EXECUTABLE(t_communicator ${t_communicator_SRC})
TARGET_LINK_LIBRARIES(t_communicator ${libs})
