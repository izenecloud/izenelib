FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS serialization filesystem system thread)

SET(_NOT_WIN32 TRUE)
IF(WIN32)
  SET(_NOT_WIN32 FALSE)
ENDIF(WIN32)

MACRO_CHECK_SHOULD_BUILD(procmeminfo _NOT_WIN32)
IF(procmeminfo_SHOULD_BUILD)
  ADD_LIBRARY(procmeminfo ProcMemInfo.cpp MRandom.cpp random.cpp profiler/Profiler.cpp profiler/TimeChecker.cpp profiler/YString.cpp)
  #INSTALL(TARGETS procmeminfo DESTINATION lib)
  INSTALL(TARGETS procmeminfo  
      LIBRARY DESTINATION lib 
      ARCHIVE DESTINATION lib 
      COMPONENT "izenelib_libraries")
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

  ADD_LIBRARY(izene_log izene_log.cpp)
  #INSTALL(TARGETS izene_log DESTINATION lib)
  INSTALL(TARGETS izene_log  
      LIBRARY DESTINATION lib 
      ARCHIVE DESTINATION lib 
      COMPONENT "izenelib_libraries")
ENDIF(izene_log_SHOULD_BUILD)


MACRO_CHECK_SHOULD_BUILD(
  izene_util
  Boost_FOUND
  )

IF(izene_util_SHOULD_BUILD)

  FILE(GLOB_RECURSE
    util_SRC
    RELATIVE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "*.cpp"
    )

  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${Glog_INCLUDE_DIRS}
  )

  ADD_DEFINITIONS("-fPIC")

  ADD_LIBRARY(izene_util ${util_SRC})

TARGET_LINK_LIBRARIES(izene_util
  ${Boost_THREAD_LIBRARY}
  ${Boost_SERIALIZATION_LIBRARY}
  ${CMAKE_THREAD_LIBS_INIT}
  )

ENDIF(izeneutil_SHOULD_BUILD)
