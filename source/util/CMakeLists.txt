IF(WIN32)

  SET_PROPERTY(GLOBAL APPEND PROPERTY NON_BUILD_COMPONENTS procmeminfo)
  MESSAGE(STATUS "! WARNING: procmeminfo can only been build in WIN32")

ELSE(WIN32)

  SET_PROPERTY(GLOBAL APPEND PROPERTY BUILD_COMPONENTS procmeminfo)

  SET(procmeminfo_SRC ProcMemInfo.cpp)

  INCLUDE(MacroAddComponent)
  MACRO_ADD_COMPONENT(procmeminfo
    "${VERSION_INSTALL_PREFIX}"
    "${PROJECT_VERSION}"
    ${procmeminfo_SRC}
    )

ENDIF(WIN32)
