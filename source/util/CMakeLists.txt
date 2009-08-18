IF(WIN32)

  SET_PROPERTY(GLOBAL APPEND PROPERTY NON_BUILD_COMPONENTS procmeminfo)
  MESSAGE(STATUS "! WARNING: procmeminfo can only been build in WIN32")

ELSE(WIN32)

  SET_PROPERTY(GLOBAL APPEND PROPERTY BUILD_COMPONENTS procmeminfo izene_log)

  SET(procmeminfo_SRC ProcMemInfo.cpp)
  SET(izene_log_SRC izene_log.cpp)

  MACRO_ADD_COMPONENT(procmeminfo
    "${VERSION_INSTALL_PREFIX}"
    "${PROJECT_VERSION}"
    ${procmeminfo_SRC}
    )
    
  MACRO_ADD_COMPONENT(izene_log
    "${VERSION_INSTALL_PREFIX}"
    "${PROJECT_VERSION}"
    ${izene_log_SRC}
    )

ENDIF(WIN32)
