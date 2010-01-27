MACRO_CHECK_SHOULD_BUILD(
  bsdmalloc
  )

IF(bsdmalloc_SHOULD_BUILD)

  SET(bsdmalloc_SRC
    "OpenBSD_malloc_Linux.c"
    )
    
  ADD_LIBRARY(bsdmalloc ${bsdmalloc_SRC})

  TARGET_LINK_LIBRARIES(
    bsdmalloc
    )
  INSTALL(TARGETS bsdmalloc     
      LIBRARY DESTINATION lib 
      COMPONENT "izenelib_libraries")

ENDIF(bsdmalloc_SHOULD_BUILD)
