IF (NOT WIN32)
  SET(PKG_CONFIG_PATH "$ENV{PKG_CONFIG_PATH}" CACHE PATH "A  colon-separated (on Windows, semicolon-separated) list of directories to search for .pc files")
  MARK_AS_ADVANCED(PKG_CONFIG_PATH)

  SET(ENV{PKG_CONFIG_PATH} ${PKG_CONFIG_PATH})
ENDIF (NOT WIN32)