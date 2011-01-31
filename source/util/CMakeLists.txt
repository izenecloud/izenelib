FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS serialization filesystem system thread regex)

SET(_NOT_WIN32 TRUE)
IF(WIN32)
  SET(_NOT_WIN32 FALSE)
ENDIF(WIN32)

add_subdirectory( compression )

MACRO_CHECK_SHOULD_BUILD(procmeminfo _NOT_WIN32)
IF(procmeminfo_SHOULD_BUILD)
  MACRO_ADD_LIBRARY(procmeminfo
    SRCS
    ProcMemInfo.cpp
    MRandom.cpp
    random.cpp
    profiler/Profiler.cpp
    profiler/TimeChecker.cpp
    profiler/YString.cpp)
ENDIF(procmeminfo_SHOULD_BUILD)

MACRO_CHECK_SHOULD_BUILD(
  izene_log
  procmeminfo_SHOULD_BUILD
  Glog_FOUND
  )
IF(izene_log_SHOULD_BUILD)
  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${Glog_INCLUDE_DIRS}
    )

  MACRO_ADD_LIBRARY(izene_log SRCS izene_log.cpp)
ENDIF(izene_log_SHOULD_BUILD)

SET(ticpp_SHOULD_BUILD TRUE)
MACRO_CHECK_SHOULD_BUILD(
  ticpp
  ticpp_SHOULD_BUILD
  )

IF(ticpp_SHOULD_BUILD)
  SET(ticpp_SRC
    ticpp/ticpp.cpp
    ticpp/tinyxmlerror.cpp
    ticpp/tinystr.cpp
    ticpp/tinyxml.cpp
    ticpp/tinyxmlparser.cpp
    )

  MACRO_ADD_LIBRARY(ticpp SRCS ${ticpp_SRC})

ENDIF(ticpp_SHOULD_BUILD)

MACRO_CHECK_SHOULD_BUILD(
  izene_util
  Boost_FOUND
  Threads_FOUND
  )

IF(izene_util_SHOULD_BUILD)

  SET(util_SRC
    scheduler.cpp

    thread-pool/ThreadObject.cpp
    thread-pool/ThreadObjectPool.cpp

    io/BufferedInput.cpp
    io/FSBufferedInput.cpp
    io/MMapBufferedInput.cpp
    io/BufferedOutput.cpp
    io/FSBufferedOutput.cpp

    sysinfo/commandparser.cpp
    sysinfo/dmiparser.cpp

    cronexpression.cpp
    timer.cpp
    CBitArray.cpp
    singleton.cpp
    streambuf.cpp
    scd_parser.cpp

    driver/Keys.cpp
    driver/Request.cpp
    driver/Response.cpp
    driver/RestrictedObjectValue.cpp
    driver/Router.cpp
    driver/value/Value.cpp
    driver/value/ValueTypeHelper.cpp
    driver/value/formatters.cpp
    driver/Reader.cpp
    driver/readers/JsonReader.cpp
    driver/Writer.cpp
    driver/writers/DriverValue2JsonValue.cpp
    driver/writers/JsonWriter.cpp
    driver/writers/PrettyJsonWriter.cpp
    driver/Parser.cpp
    driver/parsers/PageInfoParser.cpp
    driver/parsers/ConditionParser.cpp
    driver/parsers/ConditionOperatorTable.cpp
    driver/parsers/ConditionArrayParser.cpp
    driver/parsers/OrderParser.cpp
    driver/parsers/OrderArrayParser.cpp     

    )

  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${Glog_INCLUDE_DIRS}
  )

  ADD_DEFINITIONS("-fPIC")

  MACRO_ADD_LIBRARY(izene_util SRCS ${util_SRC})

  TARGET_LINK_LIBRARIES(izene_util
    ${Boost_THREAD_LIBRARY}
    ${Boost_SERIALIZATION_LIBRARY}
    ${Boost_REGEX_LIBRARY}
    json
    ${CMAKE_THREAD_LIBS_INIT}
    )

ENDIF(izene_util_SHOULD_BUILD)
