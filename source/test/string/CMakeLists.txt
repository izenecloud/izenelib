SET(t_string_SRC
  Runner.cpp

  t_RangeTokenIterator.cpp
  t_RangeTokenizer.cpp
  )

ADD_EXECUTABLE(t_string ${t_string_SRC})
TARGET_LINK_LIBRARIES(t_string
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  )

SET_TARGET_PROPERTIES(t_string PROPERTIES
  COMPILE_FLAGS "-include ${FileTestHelper}"
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/testbin
  )

ADD_TEST(string "${CMAKE_SOURCE_DIR}/testbin/t_string")
