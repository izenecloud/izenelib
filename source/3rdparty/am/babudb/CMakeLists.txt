FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS thread)

MACRO_CHECK_SHOULD_BUILD(babudb Boost_FOUND)

IF(babudb_SHOULD_BUILD)

  FILE(GLOB_RECURSE
    babudb_SRC
    RELATIVE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "*.cpp"
    )


  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${IZENELIB_SOURCE_DIR}/include/3rdparty/am/
    ${IZENELIB_SOURCE_DIR}/include/3rdparty/am/babudb
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

  ADD_DEFINITIONS("-fPIC")

  MACRO_ADD_LIBRARY(babudb SRCS ${babudb_SRC})

ENDIF(babudb_SHOULD_BUILD)

