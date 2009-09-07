FIND_PACKAGE(Boost 1.36.0
  COMPONENTS serialization filesystem system thread)

IF(wiselib_FOUND AND Boost_FOUND)

  SET_PROPERTY(GLOBAL APPEND PROPERTY BUILD_COMPONENTS index_manager)

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
    )

  ADD_DEFINITIONS("-fPIC -DUDT3")

  MACRO_ADD_COMPONENT(
    index_manager
    "${VERSION_INSTALL_PREFIX}"
    "${PROJECT_VERSION}"
    ${index_manager_SRC}
    )

  # TARGET_LINK_LIBRARIES(index_manager_shared
  #   streamflow_shared
  #   udt3_shared
  #   )
  # TARGET_LINK_LIBRARIES(index_manager_static
  #   streamflow_shared
  #   udt3_shared
  #   )

ELSE(wiselib_FOUND AND Boost_FOUND)

  SET_PROPERTY(GLOBAL APPEND PROPERTY NON_BUILD_COMPONENTS index_manager)
  MESSAGE(STATUS "! WARNING: index_manager cannot been built without wiselib and boost")

ENDIF(wiselib_FOUND AND Boost_FOUND)
