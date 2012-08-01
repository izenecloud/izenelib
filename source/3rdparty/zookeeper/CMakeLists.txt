FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS serialization filesystem system thread)
  
  FILE(GLOB_RECURSE
    zookeeper_SRC
    RELATIVE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "*.c"
    "*.cpp"
    )

  INCLUDE_DIRECTORIES(
    "${CMAKE_CURRENT_SOURCE_DIR}"
    ${Boost_INCLUDE_DIRS}
    ${IZENELIB_SOURCE_DIR}/include/3rdparty/zookeeper
    )

  CONFIGURE_FILE( 
    "${CMAKE_SOURCE_DIR}/include/3rdparty/zookeeper/config.h.in"
    "${CMAKE_SOURCE_DIR}/include/3rdparty/zookeeper/config.h"
    ESCAPE_QUOTES @ONLY
    )

  ADD_DEFINITIONS("-fPIC -DTHREADED -D_GNU_SOURCE")

  MACRO_ADD_LIBRARY(zookeeper SRCS ${zookeeper_SRC})

