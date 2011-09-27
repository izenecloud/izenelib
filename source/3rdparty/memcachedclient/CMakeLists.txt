FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS serialization filesystem system thread)
  
  FILE(GLOB
    memcachedclient_SRC
    RELATIVE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "util/*.cc"
    "libhashkit/*.cc"
    "libmemcached/csl/*.cc"
    "libmemcached/protocol/*.c"
    "libmemcached/util/*.cc"
    "libmemcached/*.cc"
    "libmemcached/*.c"
    "clients/utilities.cc"
    "*.cc"
    )

  INCLUDE_DIRECTORIES(
    "${CMAKE_CURRENT_SOURCE_DIR}"
    ${Boost_INCLUDE_DIRS}
    ${IZENELIB_SOURCE_DIR}/include/3rdparty/memcachedclient
    )

  ADD_DEFINITIONS("-fPIC -DTHREADED -D_GNU_SOURCE")

  MACRO_ADD_LIBRARY(memcachedclient SRCS ${memcachedclient_SRC})

