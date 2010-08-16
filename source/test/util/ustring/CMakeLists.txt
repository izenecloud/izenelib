# adds executable targets
ADD_EXECUTABLE(t_UString
  t_master_suite.cpp
  t_basic_interface.cpp
  t_convert_encoding_type.cpp
  t_english_char_traits.cpp
  t_find_char.cpp
  t_find_str.cpp
  t_old_tests.cpp
  t_ustr_tool.cpp
  )

# specify the libraries to link for all the targets
#equivalent to -l option
TARGET_LINK_LIBRARIES(t_UString
  wiselib
  ${izenelib_procmeminfo_LIBRARY}
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  )

# set properties for the targets
SET_TARGET_PROPERTIES(t_UString
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${wiselib_BINARY_DIR}/testbin 
  #EXCLUDE_FROM_ALL 1
  )

#test name, executable name
ADD_TEST(t_UString ${wiselib_BINARY_DIR}/testbin/t_UString ${wiselib_BINARY_DIR}/testbin)
