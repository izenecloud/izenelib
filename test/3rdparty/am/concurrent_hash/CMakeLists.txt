INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${IZENELIB_SOURCE_DIR}/include/3rdparty/am/
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
  
SET(t_concurrent_hash_SRC
  t_hash.cpp
  t_bench.cpp
  t_master_suite.cpp
  )

  ADD_EXECUTABLE(t_concurrent_hash ${t_concurrent_hash_SRC})
  TARGET_LINK_LIBRARIES(t_concurrent_hash
   izene_util
   febird
   ${Boost_LIBRARIES}
   ${Glog_LIBRARIES}
   procmeminfo
    )

