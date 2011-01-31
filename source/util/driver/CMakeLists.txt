ADD_LIBRARY(izenelib_driver
  Keys.cpp
  Request.cpp
  Response.cpp
  RestrictedObjectValue.cpp
  Router.cpp

  value/Value.cpp
  value/ValueTypeHelper.cpp
  value/formatters.cpp

  Reader.cpp
  readers/JsonReader.cpp

  Writer.cpp
  writers/DriverValue2JsonValue.cpp
  writers/JsonWriter.cpp
  writers/PrettyJsonWriter.cpp

  Parser.cpp
  parsers/PageInfoParser.cpp
  parsers/ConditionParser.cpp
  parsers/ConditionOperatorTable.cpp
  parsers/ConditionArrayParser.cpp
  parsers/OrderParser.cpp
  parsers/OrderArrayParser.cpp
  )

INSTALL(TARGETS
  izenelib_driver
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  COMPONENT sf1r_libraries
  )
