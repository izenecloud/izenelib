SET(t_string_SRC
  Runner.cpp

  t_RangeTokenIterator.cpp
  t_RangeTokenizer.cpp
  t_CharSeparator.cpp
  )

ADD_EXECUTABLE(t_string ${t_string_SRC})
TARGET_LINK_LIBRARIES(t_string
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  )

SET(t_kstring_SRC
  Runner.cpp
  t_kstring.cpp
  )

ADD_EXECUTABLE(t_kstring ${t_kstring_SRC})
TARGET_LINK_LIBRARIES(t_kstring
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  )
