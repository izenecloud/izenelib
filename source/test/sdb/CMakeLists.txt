FIND_PACKAGE(Boost 1.36 REQUIRED
  COMPONENTS
  system
  thread
  date_time
  serialization
  filesystem
  unit_test_framework
  )

INCLUDE_DIRECTORIES(
  ${izenelib_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  )

ADD_EXECUTABLE(t_sdb
  Runner.cc
  t_btree_str.cc
  t_btree1.cc
  t_btree2.cc
  t_dm.cc
  t_IndexSDB.cc
  t_indexsdb.cc
  t_IndexSDB1.cc
  t_mul_sdb.cc
  t_nsdb.cc
  t_overflow_btree.cc
  t_sdb_query.cc
  t_sdb.cc
  t_TrieIndexSDB.cc
  t_TrieIndexSDB1.cc
  )

TARGET_LINK_LIBRARIES(t_sdb

  ${Boost_IOSTREAMS_LIBRARY} 
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY} 
  filemapper
  am
  izene_util
  ${Boost_LIBRARIES}
  bz2 
  ${Glog_LIBRARIES}
  febird
  procmeminfo
  luxio
  izene_log
  ${TokyoCabinet_LIBRARIES}
  boost_iostreams
  lzo
  )


