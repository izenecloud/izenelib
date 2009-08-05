SET(streamflow_ARCH)

IF(CMAKE_SYSTEM_PROCESSOR MATCHES "i[0-9]?86")
  SET(streamflow_ARCH "x86")
  SET(streamflow_DEFINITIONS "-D_GNU_SOURCE -m32 -D_REENTRANT -Dx86 -DRADIX_TREE" )
ELSEIF(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
  SET(streamflow_ARCH "x86_64")
  SET(streamflow_DEFINITIONS "-D_GNU_SOURCE -m64 -D_REENTRANT -Dx86_64 -DRADIX_TREE")
ENDIF(CMAKE_SYSTEM_PROCESSOR MATCHES "i[0-9]?86")

IF(streamflow_ARCH AND DL_LIBRARIES AND M_LIBRARIES AND Threads_FOUND)

  MESSAGE(STATUS "Build streamflow for ${streamflow_ARCH}")
  MESSAGE(STATUS "Build extra flags ${streamflow_DEFINITIONS}")
  SET_PROPERTY(GLOBAL APPEND PROPERTY BUILD_COMPONENTS streamflow)
  SET(streamflow_SRC
    "streamflow.c"
    "override.c"
    "malloc_new.cpp"
    )

  INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/include/3rdparty/streamflow")
  INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/include/3rdparty/streamflow/include-${streamflow_ARCH}")
  ADD_DEFINITIONS("${streamflow_DEFINITIONS}")

  MACRO_ADD_COMPONENT(
    streamflow
    "${VERSION_INSTALL_PREFIX}"
    "${PROJECT_VERSION}"
    ${streamflow_SRC}
    )

  TARGET_LINK_LIBRARIES(streamflow_static
    ${CMAKE_THREAD_LIBS_INIT}
    ${M_LIBRARIES}
    ${DL_LIBRARIES}
    )
  TARGET_LINK_LIBRARIES(streamflow_shared
    ${CMAKE_THREAD_LIBS_INIT}
    ${M_LIBRARIES}
    ${DL_LIBRARIES}
    )

ELSE(streamflow_ARCH AND DL_LIBRARIES AND M_LIBRARIES AND Threads_FOUND)

  SET_PROPERTY(GLOBAL APPEND PROPERTY NON_BUILD_COMPONENTS streamflow)
  MESSAGE(STATUS "! WARNING: streamflow can only be built under x86 and x86_64 architecture now")
  MESSAGE(STATUS "! WARNING:   and it requires libraries m and dl")
  
ENDIF(streamflow_ARCH AND DL_LIBRARIES AND M_LIBRARIES AND Threads_FOUND)
