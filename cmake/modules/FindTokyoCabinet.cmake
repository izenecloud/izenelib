SET (_tokyocabinet_in_cache FALSE)
IF (TokyoCabinet_INCLUDE_DIR)
  IF (NOT TokyoCabinet_VERSION)
    FIND_FILE(FILE_TokyoCabinet_TCUTIL_H tcutil.h "${TokyoCabinet_INCLUDE_DIR}")

    IF (NOT FILE_TokyoCabinet_TCUTIL_H STREQUAL FILE_TokyoCabinet_TCUTIL_H-NOTFOUND)
      FILE(READ "${TokyoCabinet_INCLUDE_DIR}/tcutil.h" _tokyocabinet_tcutil_h_contents)
      STRING(REGEX REPLACE ".*#define _TC_VERSION[^\"]*\"([.0-9]+)\".*" "\\1" TokyoCabinet_VERSION "${_tokyocabinet_tcutil_h_contents}")
    ENDIF (NOT FILE_TokyoCabinet_TCUTIL_H STREQUAL FILE_TokyoCabinet_TCUTIL_H-NOTFOUND)
  ENDIF (NOT TokyoCabinet_VERSION)

  INCLUDE(MacroFindPackageCheckCacheVersion)
  MACRO_FIND_PACKAGE_CHECK_CACHE_VERSION(_tokyocabinet_in_cache TokyoCabinet)

  SET(_log4cpp_in_cache FALSE)

ENDIF (TokyoCabinet_INCLUDE_DIR)

IF (_tokyocabinet_in_cache)
  # in cache already
  MESSAGE(STATUS "TokyoCabinet in cache")

  SET(TokyoCabinet_FOUND TRUE)
  SET(TokyoCabinet_INCLUDE_DIRS ${TokyoCabinet_INCLUDE_DIR})
  SET(TokyoCabinet_LIBRARIES ${TokyoCabinet_LIBRARY})

ELSE (_tokyocabinet_in_cache)
  # should search it

  FIND_PATH(TokyoCabinet_INCLUDE_DIR tcutil.h)
  FIND_LIBRARY(TokyoCabinet_LIBRARY NAMES tokyocabinet)
  IF(TokyoCabinet_LIBRARY)
    SET(TokyoCabinet_LIBRARY "${TokyoCabinet_LIBRARY} -lz -lbz2 -lrt -lpthread -lm -lc")
  ENDIF(TokyoCabinet_LIBRARY)

  SET (TokyoCabinet_FOUND FALSE)
  IF (NOT WIN32)
    # tries pkg-config
    FIND_PACKAGE(PkgConfig REQUIRED)
    PKG_CHECK_MODULES(TokyoCabinet tokyocabinet)
    IF (TokyoCabinet_FOUND)
      SET(TokyoCabinet_INCLUDE_DIR ${TokyoCabinet_INCLUDE_DIRS})
      SET(TokyoCabinet_LIBRARY ${TokyoCabinet_LDFLAGS})
    ENDIF (TokyoCabinet_FOUND)
  ENDIF (NOT WIN32)

  IF (NOT TokyoCabinet_FOUND)
    MESSAGE(STATUS "NOT TokyoCabinet_FOUND")
    INCLUDE(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(TokyoCabinet TokyoCabinet_INCLUDE_DIR TokyoCabinet_LIBRARY)
    IF (TokyoCabinet_FOUND)
      FIND_FILE(FILE_TokyoCabinet_TCUTIL_H tcutil.h "${TokyoCabinet_INCLUDE_DIR}/log4cpp")
      IF (NOT FILE_TokyoCabinet_TCUTIL_H STREQUAL FILE_TokyoCabinet_TCUTIL_H-NOTFOUND)
        FILE(READ "${TokyoCabinet_INCLUDE_DIR}/tcutil.h" _tokyocabinet_tcutil_h_contents)
        STRING(REGEX REPLACE ".*#define _TC_VERSION[^\"]*\"([.0-9]+)\".*" "\\1" TokyoCabinet_VERSION "${_tokyocabinet_tcutil_h_contents}")
      ELSE (NOT FILE_TokyoCabinet_TCUTIL_H STREQUAL FILE_TokyoCabinet_TCUTIL_H-NOTFOUND)
        SET(TokyoCabinet_FOUND FALSE)
      ENDIF (NOT FILE_TokyoCabinet_TCUTIL_H STREQUAL FILE_TokyoCabinet_TCUTIL_H-NOTFOUND)
    ENDIF (TokyoCabinet_FOUND)
  ENDIF (NOT TokyoCabinet_FOUND)

  # checks version if user specified one
  SET(_details "Find TokyoCabinet: failed.")
  IF (TokyoCabinet_FOUND AND TokyoCabinet_FIND_VERSION)
    INCLUDE(MacroVersionCmp)
    MACRO_VERSION_CMP("${TokyoCabinet_VERSION}" "${TokyoCabinet_FIND_VERSION}" _cmp_result)
    IF (_cmp_result LESS 0)
      SET(_details "${_details} ${TokyoCabinet_FIND_VERSION} required but ${TokyoCabinet_VERSION} found")
      SET(TokyoCabinet_FOUND FALSE)
    ELSEIF (TokyoCabinet_FIND_VERSION_EXACT AND _cmp_result GREATER 0)
      SET(_details "${_details} exact ${TokyoCabinet_FIND_VERSION} required but ${TokyoCabinet_VERSION} found")
      SET(TokyoCabinet_FOUND FALSE)
    ENDIF (_cmp_result LESS 0)
  ENDIF (TokyoCabinet_FOUND AND TokyoCabinet_FIND_VERSION)

  IF (NOT TokyoCabinet_FOUND)
    IF (TokyoCabinet_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "${_details}")
    ELSEIF (NOT TokyoCabinet_FIND_QUIETLY)
      MESSAGE(STATUS "${_details}")
    ENDIF (TokyoCabinet_FIND_REQUIRED)
  ENDIF (NOT TokyoCabinet_FOUND)

ENDIF (_tokyocabinet_in_cache)

MARK_AS_ADVANCED(TokyoCabinet_LIBRARY TokyoCabinet_INCLUDE_DIR)
