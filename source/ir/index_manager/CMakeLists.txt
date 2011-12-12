FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS serialization filesystem system thread)

MACRO_CHECK_SHOULD_BUILD(
  index_manager
  Glog_FOUND
  Boost_FOUND
  )

IF(index_manager_SHOULD_BUILD)

  FILE(GLOB
    index_manager_SRC
    RELATIVE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "index/*.cpp"
    "store/*.cpp"
    "utility/*.cpp"
    )
  FILE(GLOB mock_index_manager_SRC RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "index/Mock*.cpp")
  LIST(REMOVE_ITEM index_manager_SRC ${mock_index_manager_SRC})

  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${IZENELIB_SOURCE_DIR}/include/3rdparty/am
    ${Glog_INCLUDE_DIRS}
    )

  ADD_DEFINITIONS("-fPIC -fno-strict-aliasing")

  MACRO_ADD_LIBRARY(index_manager SRCS ${index_manager_SRC})
  MACRO_ADD_LIBRARY(mock_index_manager index_manager SRCS ${mock_index_manager_SRC})

  TARGET_LINK_LIBRARIES(index_manager
      izene_util
    )

ENDIF(index_manager_SHOULD_BUILD)
