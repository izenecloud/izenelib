FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS serialization filesystem system thread)

MACRO_CHECK_SHOULD_BUILD(
  am
  wiselib_FOUND
  Glog_FOUND
  Boost_FOUND
  )

IF(am_SHOULD_BUILD)

  FILE(GLOB_RECURSE
    am_SRC
    RELATIVE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "filemapper/*.cpp"   
    )

  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${IZENELIB_SOURCE_DIR}/include/3rdparty/am
    ${wiselib_INCLUDE_DIRS}
    ${Glog_INCLUDE_DIRS}
    )

  ADD_DEFINITIONS("-fPIC -DUDT3")

  ADD_LIBRARY(am ${am_SRC})
  #INSTALL(TARGETS am DESTINATION lib)
  INSTALL(TARGETS am 
      LIBRARY DESTINATION lib 
      ARCHIVE DESTINATION lib 
      COMPONENT "izenelib_libraries")

ENDIF(am_SHOULD_BUILD)

MACRO_CHECK_SHOULD_BUILD(filemapper)
IF(filemapper_SHOULD_BUILD)
  ADD_LIBRARY(filemapper filemapper/persist.cpp)
  #INSTALL(TARGETS izene_log DESTINATION lib)
  INSTALL(TARGETS filemapper  
      LIBRARY DESTINATION lib 
      ARCHIVE DESTINATION lib 
      COMPONENT "izenelib_libraries")
ENDIF(filemapper_SHOULD_BUILD)

