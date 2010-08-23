FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS serialization filesystem system thread regex)

SET(_NOT_WIN32 TRUE)
IF(WIN32)
  SET(_NOT_WIN32 FALSE)
ENDIF(WIN32)


MACRO_CHECK_SHOULD_BUILD(
  izene_util_compression
  Boost_FOUND
  Threads_FOUND
  )

IF(izene_util_compression_SHOULD_BUILD)

  FILE(GLOB
    util_compression_SRC
    RELATIVE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "int/*.cpp"
    )

  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    )

  ADD_DEFINITIONS("-fPIC")

  MACRO_ADD_LIBRARY(izene_util_compression SRCS ${util_compression_SRC})

ENDIF(izene_util_compression_SHOULD_BUILD)
