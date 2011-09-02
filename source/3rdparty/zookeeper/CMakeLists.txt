  FILE(GLOB_RECURSE
    zookeeper_SRC
    RELATIVE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "*.c"
    )

  INCLUDE_DIRECTORIES(
    "${CMAKE_CURRENT_SOURCE_DIR}"
    ${IZENELIB_SOURCE_DIR}/include/3rdparty/zookeeper
    )

  ADD_DEFINITIONS("-fPIC -DTHREADED")

  MACRO_ADD_LIBRARY(zookeeper SRCS ${zookeeper_SRC})

