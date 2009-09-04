IF(TokyoCabinet_FOUND)

  SET_PROPERTY(GLOBAL APPEND PROPERTY BUILD_COMPONENTS izene_tc)

  SET(izene_tc_SRC Buffer.cpp)

  MACRO_ADD_COMPONENT(izene_tc
    "${VERSION_INSTALL_PREFIX}"
    "${PROJECT_VERSION}"
    ${izene_tc_SRC}
    )

ELSE(TokyoCabinet_FOUND)

  SET_PROPERTY(GLOBAL APPEND PROPERTY NON_BUILD_COMPONENTS izene_tc)
  MESSAGE(STATUS "! WARNING: izene_tc can not been build without tokyocabinet")

ENDIF(TokyoCabinet_FOUND)
