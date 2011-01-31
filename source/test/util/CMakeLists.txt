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
  t_ClockTimer.cpp
  t_IdMapper.cpp
  t_bzip.cpp
  t_scheduler.cpp
  t_serialization.cpp
  t_log.cpp
  t_wrapper.cpp
  )
ADD_EXECUTABLE(t_wrapper
  wrapper.cpp
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
TARGET_LINK_LIBRARIES(t_wrapper
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
  )

ADD_EXECUTABLE(t_compressor 
  Runner.cpp 
  t_compressor.cpp
  ) 

TARGET_LINK_LIBRARIES(t_compressor 
  izene_util_compression 
  ${Boost_LIBRARIES}
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
    json
    febird
    izene_util
    )


