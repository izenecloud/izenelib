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

ADD_EXECUTABLE(t_cache
  Runner.cc  
  t_izenecache.cc
  t_CacheDB.cc
  t_MCache.cc
  t_MFCache.cc
  t_mfserialize.cc
  t_MLRUCache.cc
  t_mserialize.cc
  t_mul_cache.cc 
  )

TARGET_LINK_LIBRARIES(t_cache
  izene_util
  ${Boost_LIBRARIES}
  bz2 
  ${Glog_LIBRARIES}
  febird
  procmeminfo
  izene_log
  )