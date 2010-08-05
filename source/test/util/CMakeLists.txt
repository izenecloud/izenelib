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

ADD_EXECUTABLE(t_util
  Runner.cpp
  t_ClockTimer.cpp
  t_IdMapper.cpp
  t_bzip.cpp
  t_scheduler.cpp
  )

TARGET_LINK_LIBRARIES(t_util
  izene_util
  ${Boost_LIBRARIES}
  bz2
  )
