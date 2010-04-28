SET(_NOT_WIN32 TRUE)
IF(WIN32)
  SET(_NOT_WIN32 FALSE)
ENDIF(WIN32)

MACRO_CHECK_SHOULD_BUILD(procmeminfo _NOT_WIN32)
IF(procmeminfo_SHOULD_BUILD)
  ADD_LIBRARY(procmeminfo ProcMemInfo.cpp)
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
  wiselib_FOUND
  )
IF(izene_log_SHOULD_BUILD)
  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${Glog_INCLUDE_DIRS}
    ${wiselib_INCLUDE_DIRS}
    )

  ADD_LIBRARY(izene_log izene_log.cpp)
  #INSTALL(TARGETS izene_log DESTINATION lib)
  INSTALL(TARGETS izene_log  
      LIBRARY DESTINATION lib 
      ARCHIVE DESTINATION lib 
      COMPONENT "izenelib_libraries")
ENDIF(izene_log_SHOULD_BUILD)
