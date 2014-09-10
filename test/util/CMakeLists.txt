ADD_SUBDIRECTORY(string) 

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
  ${Glog_INCLUDE_DIRS}
  )

ADD_EXECUTABLE(t_util
  Runner.cpp
#  t_ClockTimer.cpp
  t_csv.cpp
  t_IdMapper.cpp
  t_bzip.cpp
  t_scheduler.cpp
  t_serialization.cpp
  #t_log.cpp
  t_url_util.cpp
  t_id_util.cpp
  t_math.cpp
  t_criterion.cpp
  t_timsort.cpp
  t_fileno.cpp
  t_bloomfilter.cpp
  t_cuckoofilter.cpp
  t_cpuinfo.cpp
  t_levenshteinautomata.cpp
  t_kv2string.cpp
  )

TARGET_LINK_LIBRARIES(t_util
  izene_util
  ${Boost_LIBRARIES}
  ${Glog_LIBRARIES}
  ${OpenSSL_LIBRARIES}
  crypto
  bz2 
  febird
  procmeminfo
  izene_log
  bz2
  NUMA
  )

FILE(GLOB_RECURSE t_ustring_SRC "ustring/*.cpp")

ADD_EXECUTABLE(t_ustring
  Runner.cpp
  ${t_ustring_SRC}
  )

TARGET_LINK_LIBRARIES(t_ustring
  febird
  izene_util
  ${Boost_LIBRARIES}
  ${Glog_LIBRARIES}
  )

ADD_EXECUTABLE(t_compressor 
  Runner.cpp 
  t_compressor.cpp
  t_fastpfor.cpp
  t_compressedset.cpp
  ) 

TARGET_LINK_LIBRARIES(t_compressor 
  febird
  izene_util 
  ${Boost_LIBRARIES}
  ${Glog_LIBRARIES}
  )

ADD_EXECUTABLE(t_compressed_vector
  Runner.cpp 
  compressed_vector/t_compressed_vector.cpp
  ) 

TARGET_LINK_LIBRARIES(t_compressed_vector 
  febird
  izene_util 
  ${Boost_LIBRARIES}
  ${Glog_LIBRARIES}
  )


ADD_EXECUTABLE(t_driver
    driver/Runner.cpp
    driver/value/t_as.cpp
    driver/value/t_boost_serialization.cpp
    driver/value/t_clear.cpp
    driver/value/t_compare.cpp
    driver/value/t_febird_serialization.cpp
    driver/value/t_get.cpp
    driver/value/t_size.cpp
    driver/value/t_Value.cpp
    driver/value/t_ValueTypeHelper.cpp
    )
TARGET_LINK_LIBRARIES(t_driver
    izene_util
    ${Boost_LIBRARIES}
    ${Glog_LIBRARIES}
    json
    febird
    )
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${IZENELIB_SOURCE_DIR}/testbin")

ADD_LIBRARY(bundle1
    osgi/bundle1/BundleActivator1.cpp
    osgi/bundle1/Library.cpp
    )

ADD_LIBRARY(bundle2
    osgi/bundle2/BundleActivator2.cpp
    osgi/bundle2/Library.cpp
    )

ADD_EXECUTABLE(t_osgi
    osgi/Runner.cpp
    osgi/TestHelper.cpp
    osgi/IServiceBImpl.cpp
    osgi/TestBundleActivator.cpp
    osgi/LauncherTest.cpp
    )
TARGET_LINK_LIBRARIES(t_osgi
    febird
    izene_util
    ${Boost_LIBRARIES}
    ${Glog_LIBRARIES}    
    json
    )

ADD_EXECUTABLE(t_class_register
  Runner.cpp
  t_class_register
  )

TARGET_LINK_LIBRARIES(t_class_register
    ${Boost_LIBRARIES}
  )


ADD_EXECUTABLE(t_fmath
  fmath/t_fmath.cpp
  )

TARGET_LINK_LIBRARIES(t_fmath
    izene_util
    febird
    ${Boost_LIBRARIES}
    ${Glog_LIBRARIES}    
  )
