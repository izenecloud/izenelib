# - Find log4cpp
# Find the log4cpp headers and libraries
#
#  LOG4CPP_FOUND        - true if log4cpp found.
#  LOG4CPP_INCLUDE_DIRS - the log4cpp include directories
#  LOG4CPP_LIBRARIES    - the libraries needed to use log4cpp
#  LOG4CPP_VERSION      - version of log4cpp found
#
# Following variables are also defined, but they can also be edit by
# user in cache to override log4cpp compilation settings
#  LOG4CPP_INCLUDE_DIR
#  LOG4CPP_LIBRARY
#
SET(_log4cpp_in_cache FALSE)
IF (LOG4CPP_INCLUDE_DIR)
  IF (NOT LOG4CPP_VERSION)
    FIND_FILE(FILE_LOG4CPP_CONFIG_H config.h "${LOG4CPP_INCLUDE_DIR}/log4cpp")

    IF (NOT FILE_LOG4CPP_CONFIG_H STREQUAL FILE_LOG4CPP_CONFIG_H-NOTFOUND)
      FILE(READ "${LOG4CPP_INCLUDE_DIR}/log4cpp/config.h" _log4cpp_config_h_contents)
      STRING(REGEX REPLACE ".*#define LOG4CPP_VERSION \"([.0-9]+)\".*" "\\1" LOG4CPP_VERSION "${_log4cpp_config_h_contents}")
    ENDIF (NOT FILE_LOG4CPP_CONFIG_H STREQUAL FILE_LOG4CPP_CONFIG_H-NOTFOUND)

  ENDIF (NOT LOG4CPP_VERSION)

  INCLUDE(MacroFindPackageCheckCacheVersion)
  MACRO_FIND_PACKAGE_CHECK_CACHE_VERSION(_log4cpp_in_cache Log4Cpp)

  SET(_log4cpp_in_cache FALSE)
ENDIF (LOG4CPP_INCLUDE_DIR)

IF (_log4cpp_in_cache)
  # in cache already
  MESSAGE(STATUS "log4cpp in cache")

  SET(LOG4CPP_FOUND TRUE)
  SET(LOG4CPP_INCLUDE_DIRS ${LOG4CPP_INCLUDE_DIR})
  SET(LOG4CPP_LIBRARIES ${LOG4CPP_LIBRARY})

ELSE (_log4cpp_in_cache)
  # should search it

  FIND_PATH(LOG4CPP_INCLUDE_DIR log4cpp/config.h)
  FIND_LIBRARY(LOG4CPP_LIBRARY NAMES log4cpp)

  SET (LOG4CPP_FOUND FALSE)
  IF (NOT WIN32)
    # tries pkg-config
    FIND_PACKAGE(PkgConfig REQUIRED)
    PKG_CHECK_MODULES(LOG4CPP log4cpp)
    IF (LOG4CPP_FOUND)
      SET(LOG4CPP_INCLUDE_DIR ${LOG4CPP_INCLUDE_DIRS})
      SET(LOG4CPP_LIBRARY ${LOG4CPP_LIBRARIES})
    ENDIF (LOG4CPP_FOUND)
  ENDIF (NOT WIN32)

  IF (NOT LOG4CPP_FOUND)
    INCLUDE(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(LOG4CPP LOG4CPP_INCLUDE_DIR LOG4CPP_LIBRARY)
    IF (LOG4CPP_FOUND)
      FIND_FILE(FILE_LOG4CPP_CONFIG_H config.h "${LOG4CPP_INCLUDE_DIR}/log4cpp")
      IF (NOT FILE_LOG4CPP_CONFIG_H STREQUAL FILE_LOG4CPP_CONFIG_H-NOTFOUND)
        FILE(READ "${LOG4CPP_INCLUDE_DIR}/log4cpp/config.h" _log4cpp_config_h_contents)
        STRING(REGEX REPLACE ".*#define LOG4CPP_VERSION \"([.0-9]+)\".*" "\\1" LOG4CPP_VERSION "${_log4cpp_config_h_contents}")
      ELSE (NOT FILE_LOG4CPP_CONFIG_H STREQUAL FILE_LOG4CPP_CONFIG_H-NOTFOUND)
        SET(LOG4CPP_FOUND FALSE)
      ENDIF (NOT FILE_LOG4CPP_CONFIG_H STREQUAL FILE_LOG4CPP_CONFIG_H-NOTFOUND)
    ENDIF (LOG4CPP_FOUND)
  ENDIF (NOT LOG4CPP_FOUND)

  # checks version if user specified one
  SET(_details "Find log4cpp: failed.")
  IF (LOG4CPP_FOUND AND Log4Cpp_FIND_VERSION)
    INCLUDE(MacroVersionCmp)
    MACRO_VERSION_CMP("${LOG4CPP_VERSION}" "${Log4Cpp_FIND_VERSION}" _cmp_result)
    IF (_cmp_result LESS 0)
      SET(_details "${_details} ${Log4Cpp_FIND_VERSION} required but ${LOG4CPP_VERSION} found")
      SET(LOG4CPP_FOUND FALSE)
    ELSEIF (Log4Cpp_FIND_VERSION_EXACT AND _cmp_result GREATER 0)
      SET(_details "${_details} exact ${Log4Cpp_FIND_VERSION} required but ${LOG4CPP_VERSION} found")
      SET(LOG4CPP_FOUND FALSE)
    ENDIF (_cmp_result LESS 0)
  ENDIF (LOG4CPP_FOUND AND Log4Cpp_FIND_VERSION)

  IF (NOT LOG4CPP_FOUND)
    IF (Log4Cpp_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "${_details}")
    ELSEIF (NOT Log4Cpp_FIND_QUIETLY)
      MESSAGE(STATUS "${_details}")
    ENDIF (Log4Cpp_FIND_REQUIRED)
  ENDIF (NOT LOG4CPP_FOUND)

ENDIF (_log4cpp_in_cache)

MARK_AS_ADVANCED(LOG4CPP_LIBRARY LOG4CPP_INCLUDE_DIR)
