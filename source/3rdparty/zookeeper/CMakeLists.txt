  FILE(GLOB_RECURSE
    zookeeper_SRC
    RELATIVE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "*.c"
    "*.cpp"
    "dsync/*.cpp"
    )

  INCLUDE_DIRECTORIES(
    "${CMAKE_CURRENT_SOURCE_DIR}"
    ${IZENELIB_SOURCE_DIR}/include/3rdparty/zookeeper
    )

  ADD_DEFINITIONS("-fPIC -DTHREADED -D_GNU_SOURCE")

  MACRO_ADD_LIBRARY(zookeeper SRCS ${zookeeper_SRC})

