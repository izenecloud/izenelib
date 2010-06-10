MACRO_CHECK_SHOULD_BUILD(json)

IF(json_SHOULD_BUILD)
  SET(json_SRC
    json_reader.cpp
    json_value.cpp
    json_writer.cpp)

  ADD_LIBRARY(json ${json_SRC})
  INSTALL(TARGETS json
    LIBRARY DESTINATION lib 
    ARCHIVE DESTINATION lib 
    COMPONENT "izenelib_libraries")

ENDIF(json_SHOULD_BUILD)