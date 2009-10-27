SET(streamflow_ARCH)
IF(CMAKE_SYSTEM_PROCESSOR MATCHES "i[0-9]?86")
  SET(streamflow_ARCH "x86")
  SET(streamflow_DEFINITIONS "-D_GNU_SOURCE -m32 -D_REENTRANT -Dx86 -DHEADERS" )
ELSEIF(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
  SET(streamflow_ARCH "x86_64")
  SET(streamflow_DEFINITIONS "-D_GNU_SOURCE -m64 -D_REENTRANT -Dx86_64 -DRADIX_TREE")
ENDIF(CMAKE_SYSTEM_PROCESSOR MATCHES "i[0-9]?86")

MACRO_CHECK_SHOULD_BUILD(
  streamflow
  streamflow_ARCH
  DL_LIBRARIES
  M_LIBRARIES
  Threads_FOUND
  )

IF(streamflow_SHOULD_BUILD)

  MESSAGE(STATUS "Build streamflow for ${streamflow_ARCH}")
  MESSAGE(STATUS "Build extra flags ${streamflow_DEFINITIONS}")
  SET(streamflow_SRC
    "streamflow.c"
    "override.c"
    "malloc_new.cpp"
    )

  INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/include/3rdparty/streamflow")
  INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/include/3rdparty/streamflow/include-${streamflow_ARCH}")
  ADD_DEFINITIONS("${streamflow_DEFINITIONS}")

  ADD_LIBRARY(streamflow ${streamflow_SRC})
  TARGET_LINK_LIBRARIES(
    streamflow
    ${CMAKE_THREAD_LIBS_INIT}
    ${M_LIBRARIES}
    ${DL_LIBRARIES}
    )
  INSTALL(TARGETS streamflow DESTINATION lib)

ENDIF(streamflow_SHOULD_BUILD)
