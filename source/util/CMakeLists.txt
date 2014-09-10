FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS serialization filesystem system thread regex unit_test_framework)

SET(_NOT_WIN32 TRUE)
IF(WIN32)
  SET(_NOT_WIN32 FALSE)
ENDIF(WIN32)

MACRO_CHECK_SHOULD_BUILD(procmeminfo _NOT_WIN32)
IF(procmeminfo_SHOULD_BUILD)
  MACRO_ADD_LIBRARY(procmeminfo
    SRCS
    ProcMemInfo.cpp
    MRandom.cpp
    random_utils.cpp
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

  FILE(GLOB_RECURSE
    util_SRC

    compression/*.cpp
    compressed_vector/*.cpp
    sysinfo/*.cpp

    scheduler.cpp
    mkgmtime.cpp
    block_pool.cpp
    cronexpression.cpp
    timer.cpp
    CBitArray.cpp
    singleton.cpp
    streambuf.cpp
    stringprintf.cpp
    DynamicLibrary.cpp
    modp_numtoa.cpp
    timestamp.cpp
    mem_pool.cpp
    kv2string.cpp
    csv.cpp

    driver/*.cpp
    osgi/*.cpp
    datastream/*.cc

    string/StringUtils.cpp

    test/BoostTestThreadSafety.cpp
    )

  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${Glog_INCLUDE_DIRS}
  )

  ADD_DEFINITIONS("-fPIC")

  MACRO_ADD_LIBRARY(izene_util SRCS ${util_SRC})

  TARGET_LINK_LIBRARIES(izene_util
    ${Boost_LIBRARIES}
    json
    ${CMAKE_THREAD_LIBS_INIT}
    dl
    )

ENDIF(izene_util_SHOULD_BUILD)
