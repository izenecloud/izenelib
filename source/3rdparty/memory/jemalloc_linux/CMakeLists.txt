SET(jemalloc_DEFINITIONS "-DPIC -fPIC -DMALLOC_PRODUCTION -D__isthreaded=true" )

MACRO_CHECK_SHOULD_BUILD(
  jemalloc
  )

IF(jemalloc_SHOULD_BUILD)

  MESSAGE(STATUS "Build extra flags ${jemalloc_DEFINITIONS}")
  SET(jemalloc_SRC
    "jemalloc_linux.c"
    )
    
  ADD_DEFINITIONS("${jemalloc_DEFINITIONS}")
  
  ADD_LIBRARY(jemalloc ${jemalloc_SRC})

  TARGET_LINK_LIBRARIES(
    jemalloc
    pthread
    )
  INSTALL(TARGETS jemalloc     
      LIBRARY DESTINATION lib 
      COMPONENT "izenelib_libraries")

ENDIF(jemalloc_SHOULD_BUILD)
