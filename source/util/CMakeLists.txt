IF(WIN32)

  SET_PROPERTY(GLOBAL APPEND PROPERTY NON_BUILD_COMPONENTS procmeminfo)
  MESSAGE(STATUS "! WARNING: procmeminfo can only been build in WIN32")

ELSE(WIN32)

  SET_PROPERTY(GLOBAL APPEND PROPERTY BUILD_COMPONENTS procmeminfo)

  SET(procmeminfo_SRC ProcMemInfo.cpp)

  ADD_LIBRARY(procmeminfo_shared SHARED ${procmeminfo_SRC})
  SET_TARGET_PROPERTIES(procmeminfo_shared PROPERTIES
    OUTPUT_NAME "procmeminfo"
    CLEAN_DIRECT_OUTPUT 1
    VERSION ${IZENELIB_VERSION}
    )

  ADD_LIBRARY(procmeminfo_static STATIC ${procmeminfo_SRC})
  SET_TARGET_PROPERTIES(procmeminfo_static PROPERTIES
    OUTPUT_NAME "procmeminfo"
    CLEAN_DIRECT_OUTPUT 1
    )

  INSTALL(TARGETS procmeminfo_static procmeminfo_shared
    EXPORT procmeminfo
    ARCHIVE DESTINATION ${VERSION_INSTALL_PREFIX}/lib
    LIBRARY DESTINATION shared)

  INSTALL(EXPORT procmeminfo
    DESTINATION ${VERSION_INSTALL_PREFIX}/cmake)

ENDIF(WIN32)
