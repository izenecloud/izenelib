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

SET(t_leveldb_SRC
  t_leveldb.cpp
  t_master_suite.cpp
  )

ADD_EXECUTABLE(t_leveldb ${t_leveldb_SRC})
TARGET_LINK_LIBRARIES(t_leveldb
  leveldb
  compressor
  febird
  izene_util
  ${Boost_LIBRARIES}
  ${Glog_LIBRARIES}
  procmeminfo
  )
