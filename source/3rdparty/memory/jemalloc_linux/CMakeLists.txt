MACRO_CHECK_SHOULD_BUILD(jemalloc)

IF(jemalloc_SHOULD_BUILD)

  SET(jemalloc_DEFINITIONS "-DPIC -fPIC -DMALLOC_PRODUCTION -D__isthreaded=true")
  ADD_DEFINITIONS("${jemalloc_DEFINITIONS}")
  MESSAGE(STATUS "jemalloc_DEFINITIONS=${jemalloc_DEFINITIONS}")

  MACRO_ADD_LIBRARY(jemalloc SRCS "jemalloc_linux.c")

  TARGET_LINK_LIBRARIES(jemalloc pthread)

ENDIF(jemalloc_SHOULD_BUILD)
