SET(nedmalloc_DEFINITIONS "-DREPLACE_SYSTEM_ALLOCATOR" )

MACRO_CHECK_SHOULD_BUILD(
  nedmalloc
  )

IF(nedmalloc_SHOULD_BUILD)

  MESSAGE(STATUS "Build extra flags ${streamflow_DEFINITIONS}")
  SET(nedmalloc_SRC
    "nedmalloc.c"
    )
  INCLUDE_DIRECTORIES("${IZENELIB_SOURCE_DIR}/include/3rdparty/nedmalloc")
  ADD_DEFINITIONS("${nedmalloc_DEFINITIONS}")
  ADD_LIBRARY(nedmalloc ${nedmalloc_SRC})

  TARGET_LINK_LIBRARIES(
    nedmalloc
    )
  INSTALL(TARGETS nedmalloc     
      LIBRARY DESTINATION lib 
      COMPONENT "izenelib_libraries")

ENDIF(nedmalloc_SHOULD_BUILD)
