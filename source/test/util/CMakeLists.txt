ADD_EXECUTABLE(t_util
  Runner.cpp
  t_ClockTimer.cpp
  t_IdMapper.cpp
  )
TARGET_LINK_LIBRARIES(t_util ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
SET_TARGET_PROPERTIES(t_util PROPERTIES
  COMPILE_FLAGS "-include ${FileTestHelper}"
  RUNTIME_OUTPUT_DIRECTORY ${IZENELIB_SOURCE_DIR}/testbin
  )
ADD_TEST(util "${IZNELIB_SOURCE_DIR}/testbin/t_util")
