FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS serialization filesystem system thread)

MACRO_CHECK_SHOULD_BUILD(
  index_manager
  wiselib_FOUND
  Glog_FOUND
  Boost_FOUND
  )

IF(index_manager_SHOULD_BUILD)

  FILE(GLOB_RECURSE
    index_manager_SRC
    RELATIVE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "index/*.cpp"
    "store/*.cpp"
    "utility/*.cpp"
    )

  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include/3rdparty/am
    ${wiselib_INCLUDE_DIRS}
    ${Glog_INCLUDE_DIRS}
    )

  ADD_DEFINITIONS("-fPIC -DUDT3")

  ADD_LIBRARY(index_manager ${index_manager_SRC})
  #INSTALL(TARGETS index_manager DESTINATION lib)
  INSTALL(TARGETS index_manager 
      LIBRARY DESTINATION lib 
      ARCHIVE DESTINATION lib 
      COMPONENT "izenelib_libraries")

ENDIF(index_manager_SHOULD_BUILD)
