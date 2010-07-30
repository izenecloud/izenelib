MACRO_CHECK_SHOULD_BUILD(json)

IF(json_SHOULD_BUILD)
  SET(json_SRC
    json_reader.cpp
    json_value.cpp
    json_writer.cpp)

  MACRO_ADD_LIBRARY(json SRCS ${json_SRC})

ENDIF(json_SHOULD_BUILD)