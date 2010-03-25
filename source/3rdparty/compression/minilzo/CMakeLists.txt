SET(lzo_DEFINITIONS "-DPIC -fPIC -DMALLOC_PRODUCTION -D__isthreaded=true" )

MACRO_CHECK_SHOULD_BUILD(
  lzo
  )

IF(lzo_SHOULD_BUILD)

  MESSAGE(STATUS "Build extra flags ${lzo_DEFINITIONS}")
  SET(lzo_SRC
    "minilzo.c"
    )

 INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/include/3rdparty/compression/minilzo
    )
    
  ADD_DEFINITIONS("${lzo_DEFINITIONS}")
  
  ADD_LIBRARY(lzo ${lzo_SRC})

  TARGET_LINK_LIBRARIES(
    lzo
    )
  INSTALL(TARGETS lzo     
      LIBRARY DESTINATION lib 
      ARCHIVE DESTINATION lib 
      COMPONENT "izenelib_libraries")

ENDIF(lzo_SHOULD_BUILD)
