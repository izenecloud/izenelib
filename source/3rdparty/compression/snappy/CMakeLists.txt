MACRO_CHECK_SHOULD_BUILD(snappy)

IF(snappy_SHOULD_BUILD)

  FILE(GLOB_RECURSE
    snappy_SRC
    RELATIVE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "*.cc"
    )


  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${IZENELIB_SOURCE_DIR}/include/3rdparty/
    ${IZENELIB_SOURCE_DIR}/include/3rdparty/compression/snappy
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

  ADD_DEFINITIONS("-fPIC")

  MACRO_ADD_LIBRARY(snappy SRCS ${snappy_SRC})

ENDIF(snappy_SHOULD_BUILD)

