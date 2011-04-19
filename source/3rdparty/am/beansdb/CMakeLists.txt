FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS thread)

MACRO_CHECK_SHOULD_BUILD(beansdb Boost_FOUND)

IF(beansdb_SHOULD_BUILD)

  FILE(GLOB_RECURSE
    beansdb_SRC
    RELATIVE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "*.c"
    )

  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${IZENELIB_SOURCE_DIR}/include/3rdparty/am/beansdb
    )

  ADD_DEFINITIONS("-fPIC")

  MACRO_ADD_LIBRARY(beansdb SRCS ${beansdb_SRC})

ENDIF(beansdb_SHOULD_BUILD)

