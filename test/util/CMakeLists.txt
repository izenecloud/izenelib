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
  )

TARGET_LINK_LIBRARIES(t_util
  izene_util
  ${Boost_LIBRARIES}
  bz2 
  ${Glog_LIBRARIES}
  febird
  procmeminfo
  izene_log
  bz2
  )

FILE(GLOB_RECURSE t_ustring_SRC "ustring/*.cpp")

ADD_EXECUTABLE(t_ustring
  Runner.cpp
  ${t_ustring_SRC}
  )

TARGET_LINK_LIBRARIES(t_ustring
  izene_util
  ${Boost_LIBRARIES}
  ${Glog_LIBRARIES}
  )

ADD_EXECUTABLE(t_mempool
  Runner.cpp
  t_mempool.cpp
  )

TARGET_LINK_LIBRARIES(t_mempool
  izene_util
  ${Boost_LIBRARIES}
  ${Glog_LIBRARIES}
  )

ADD_EXECUTABLE(t_compressor 
  Runner.cpp 
  t_compressor.cpp
  ) 

TARGET_LINK_LIBRARIES(t_compressor 
  izene_util 
  ${Boost_LIBRARIES}
  ${Glog_LIBRARIES}
  )

ADD_EXECUTABLE(t_compressed_vector
  Runner.cpp 
  compressed_vector/t_compressed_vector.cpp
  ) 

TARGET_LINK_LIBRARIES(t_compressed_vector 
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
    ${Boost_LIBRARIES}
    ${Glog_LIBRARIES}
    json
    febird
    izene_util
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
    ${Boost_LIBRARIES}
    ${Glog_LIBRARIES}    
    json
    izene_util
    )

ADD_EXECUTABLE(t_class_register
  Runner.cpp
  t_class_register
  )

TARGET_LINK_LIBRARIES(t_class_register
    ${Boost_LIBRARIES}
  )

ADD_EXECUTABLE(scd_tool
  scd_tool.cpp
  )

TARGET_LINK_LIBRARIES(scd_tool
    ${Boost_LIBRARIES}
    izene_util
    ${Glog_LIBRARIES}    
  )
