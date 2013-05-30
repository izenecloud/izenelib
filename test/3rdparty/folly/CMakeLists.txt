INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${IZENELIB_SOURCE_DIR}/include/3rdparty/folly/
    )
FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS
  system
  thread
  filesystem
  serialization
  unit_test_framework
  program_options
  )
  
SET(t_concurrent_skip_list_SRC
  t_skiplist.cpp
#  t_bench.cpp
  t_master_suite.cpp
  )

ADD_EXECUTABLE(t_concurrent_skip_list ${t_concurrent_skip_list_SRC})
TARGET_LINK_LIBRARIES(t_concurrent_skip_list
   izene_util
   febird
   ${Boost_LIBRARIES}
   ${Glog_LIBRARIES}
   procmeminfo
    )

