MACRO_CHECK_SHOULD_BUILD(jemalloc2)

IF(jemalloc2_SHOULD_BUILD)

  SET(jemalloc2_DEFINITIONS "-std=gnu99 -Wall -pipe -g3 -fvisibility=hidden -O3 -funroll-loops -fPIC -DPIC -c -D_GNU_SOURCE -D_REENTRANT")
  ADD_DEFINITIONS("${jemalloc2_DEFINITIONS}")
  MESSAGE(STATUS "jemalloc2_DEFINITIONS=${jemalloc2_DEFINITIONS}")

  INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

  FILE(GLOB
    jemalloc2_SRC
    RELATIVE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "src/*.c"
    )

  LIST(REMOVE_ITEM jemalloc2_SRC "src/zone.c")

  MACRO_ADD_LIBRARY(jemalloc2 SRCS ${jemalloc2_SRC})

  TARGET_LINK_LIBRARIES(jemalloc2 pthread)
ENDIF(jemalloc2_SHOULD_BUILD)
