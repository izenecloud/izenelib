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
SET_TARGET_PROPERTIES(t_util PROPERTIES
  COMPILE_FLAGS "-include ${FileTestHelper}"
  RUNTIME_OUTPUT_DIRECTORY ${IZENELIB_SOURCE_DIR}/testbin
  )
ADD_TEST(util "${IZENELIB_SOURCE_DIR}/testbin/t_util")
