SET(lmdb_DEFINITIONS "-std=gnu99 -W -Wall -Wno-unused-parameter -Wbad-function-cast -g -O2 -fPIC")
ADD_DEFINITIONS("${lmdb_DEFINITIONS}")
MESSAGE(STATUS "lmdb_DEFINITIONS=${lmdb_DEFINITIONS}")
FILE(GLOB_RECURSE
  lmdb_SRC
  RELATIVE
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "*.c"
  )

INCLUDE_DIRECTORIES(
  ${IZENELIB_SOURCE_DIR}/include/3rdparty/am/lmdb
  )

MACRO_ADD_LIBRARY(lmdb SRCS ${lmdb_SRC})


