MACRO_CHECK_SHOULD_BUILD(jemalloc)

IF(jemalloc_SHOULD_BUILD)

  SET(jemalloc_DEFINITIONS "-std=gnu99 -Wall -pipe -g3 -fvisibility=hidden -O3 -funroll-loops -fPIC -DPIC -c -D_GNU_SOURCE -D_REENTRANT")
  ADD_DEFINITIONS("${jemalloc_DEFINITIONS}")
  MESSAGE(STATUS "jemalloc_DEFINITIONS=${jemalloc_DEFINITIONS}")

  INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

  FILE(GLOB
    jemalloc_SRC
    RELATIVE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "src/*.c"
    )

  LIST(REMOVE_ITEM jemalloc_SRC "src/zone.c")

  MACRO_ADD_LIBRARY(jemalloc SRCS ${jemalloc_SRC})

  TARGET_LINK_LIBRARIES(jemalloc pthread)
ENDIF(jemalloc_SHOULD_BUILD)
