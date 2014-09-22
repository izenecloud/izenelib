CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

##################################################
# Find extra cmake modules dirs
#
# You can clone all modules from https://ssl.izenesoft.cn/projects/cmake
#
# Clone it and set environment variable EXTRA_CMAKE_MODULES_DIRS point to it.
#
SET(ENV_EXTRA_CMAKE_MODULES_DIRS $ENV{EXTRA_CMAKE_MODULES_DIRS})
IF(ENV_EXTRA_CMAKE_MODULES_DIRS)
  SET(EXTRA_CMAKE_MODULES_DIRS ${EXTRA_CMAKE_MODULES_DIRS} ${ENV_EXTRA_CMAKE_MODULES_DIRS})
ENDIF(ENV_EXTRA_CMAKE_MODULES_DIRS)
IF(NOT EXTRA_CMAKE_MODULES_DIRS)
  MESSAGE(FATAL_ERROR
    "Cannot find EXTRA_CMAKE_MODULES_DIRS.
1. Please clone it from https://ssl.izenesoft.cn/cmake.
2. Then set EXTRA_CMAKE_MODULES_DIRS through environment variable

       export EXTRA_CMAKE_MODULES_DIRS=/path/to/cmake

   You can put it in your .bashrc so it is loaded automatically.

   or cmake options

       cmake  -DEXTRA_CMAKE_MODULES_DIRS=/path/to/cmake (other options)
")
ENDIF(NOT EXTRA_CMAKE_MODULES_DIRS)
SET(CMAKE_MODULE_PATH ${EXTRA_CMAKE_MODULES_DIRS})

# force out-of-source build
IF (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
  MESSAGE(FATAL_ERROR "In source build is not allowed")
ENDIF (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)

##################################################
# Project and some options
#####
PROJECT(IZENELIB)
OPTION(VERBOSE "print detailed configure messages" ON)
OPTION(ENABLE_TEST "build test applications?" ON)

##################################################
# BUILD TYPE & OPTIONS
#
# Put this section just under PROJECT
#####

# Use BUILD_FLAGS_HAS_BEEN_CUSTOMIZED as guard.
#
# The options are only customized in first run. User can edit cache to
# set their own options. If it's required to customized these in cmake
# command line options, please also add
# "-DBUILD_FLAGS_HAS_BEEN_CUSTOMIZED".
IF (CMAKE_COMPILER_IS_GNUCXX)
  INCLUDE(MacroSetBuildTypes)
  MACRO_SET_BUILD_TYPES(
    RelWithDebInfo
    "-march=native -mtune=native -O2 -g -DNDEBUG -std=c++11"
    "Flags used by the compiler during Release with Debug Info builds."

    Release
    "-march=native -mtune=native -O3 -DNDEBUG -std=c++11"
    "Flags used by the compiler during release builds"

    MinSizeRel
    "-march=native -mtune=native -Os -DNDEBUG -std=c++11"
    "Flags used by the compiler during release minsize builds."

    Debug
    "-march=native -mtune=native -g -fno-reorder-blocks -fno-schedule-insns -fno-inline -std=c++11"
    "Flags used by the compiler during debug builds."

    DebugFull
    "-march=native -mtune=native -g3 -fno-inline -std=c++11"
    "Debugfull build type"

    Profile
    "-march=native -mtune=native -g -O0 -fno-inline -ftest-coverage -fprofile-arcs -std=c++11"
    "Profile build type"
    )
ENDIF (CMAKE_COMPILER_IS_GNUCXX)

INCLUDE(MacroSetDefaultBuildType)
MACRO_SET_DEFAULT_BUILD_TYPE(RelWithDebInfo)

INCLUDE(SetCompilerWarningAll)
# Write to cache
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE STRING
  "Flags used by the compiler during all build types." FORCE)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING
  "Flags used by the compiler during all build types." FORCE)

# build shared libs by default
OPTION(BUILD_SHARED_LIBS "Build shared libraries?" ON)

##################################################
# Check Dependencies
####
INCLUDE(SetPkgConfigPath)
INCLUDE(ConfigureChecks.cmake)

##################################################
# Install Headers
#####

# trailing / is required, otherwise dectory name is appended to
# install directory.
INSTALL(
    DIRECTORY include/
    DESTINATION include
    COMPONENT "izenelib_headers"
    )

##################################################
# Sub Directories
#####

# set output directory
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${IZENELIB_SOURCE_DIR}/lib")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${IZENELIB_SOURCE_DIR}/lib")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${IZENELIB_SOURCE_DIR}/bin")

# all headers
INCLUDE_DIRECTORIES("${IZENELIB_SOURCE_DIR}/include")
INCLUDE_DIRECTORIES("${IZENELIB_SOURCE_DIR}/include/3rdparty")

# macros
INCLUDE(MacroCheckShouldBuild)
INCLUDE(MacroAddLibrary)

# third party libraries
# put it first, they may be referred in other components
ADD_SUBDIRECTORY(source/3rdparty 3rdparty)
ADD_SUBDIRECTORY(source/am am)
ADD_SUBDIRECTORY(source/ir ir)
ADD_SUBDIRECTORY(source/util util)
ADD_SUBDIRECTORY(source/net net)
ADD_SUBDIRECTORY(source/sf1common sf1common)

GET_PROPERTY(REAL_BUILD_COMPONENTS GLOBAL PROPERTY BUILD_COMPONENTS)
GET_PROPERTY(REAL_NON_BUILD_COMPONENTS GLOBAL PROPERTY NON_BUILD_COMPONENTS)

# Test cases and applications
IF(ENABLE_TEST)
  ADD_SUBDIRECTORY(test test)
ENDIF(ENABLE_TEST)

##################################################
# Messages
#####
IF(VERBOSE)

  MESSAGE(STATUS "========================================")
  MESSAGE(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
  MESSAGE(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
  STRING(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)
  MESSAGE(STATUS "  corresponding flags: ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE_UPPER}}")
  MESSAGE(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
  MESSAGE(STATUS "CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")
  MESSAGE(STATUS "----------------------------------------")

  IF(Boost_FOUND)
    MESSAGE(STATUS "BOOST:")
    MESSAGE(STATUS "  header: ${Boost_INCLUDE_DIR}")
    MESSAGE(STATUS "  version: ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")
  ELSE(Boost_FOUND)
    MESSAGE(STATUS "BOOST: not found")
  ENDIF(Boost_FOUND)

  IF(BZIP2_FOUND)
    MESSAGE(STATUS "BZIP2:")
    MESSAGE(STATUS "  header: ${BZIP2_INCLUDE_DIR}")
    MESSAGE(STATUS "  libs: ${BZIP2_LIBRARIES}")
  ELSE(BZIP2_FOUND)
    MESSAGE(STATUS "BZIP2: not found")
  ENDIF(BZIP2_FOUND)

  IF(ZLIB_FOUND)
    MESSAGE(STATUS "ZLIB:")
    MESSAGE(STATUS "  header: ${ZLIB_INCLUDE_DIR}")
    MESSAGE(STATUS "  libs: ${ZLIB_LIBRARIES}")
  ELSE(ZLIB_FOUND)
    MESSAGE(STATUS "ZLIB: not found")
  ENDIF(ZLIB_FOUND)

  IF(Threads_FOUND)
    MESSAGE(STATUS "Threads Library: ${CMAKE_THREAD_LIBS_INIT}")
  ELSE(Threads_FOUND)
    MESSAGE(STATUS "Threads Library: none")
  ENDIF(Threads_FOUND)

  IF(TokyoCabinet_FOUND)
    MESSAGE(STATUS "Tokyo Cabinet:")
    MESSAGE(STATUS "  header: ${TokyoCabinet_INCLUDE_DIRS}")
    MESSAGE(STATUS "  libs: ${TokyoCabinet_LIBRARIES}")
    MESSAGE(STATUS "  version: ${TokyoCabinet_VERSION}")
  ELSE(TokyoCabinet_FOUND)
    MESSAGE(STATUS "TokyoCabinet: not found")
  ENDIF(TokyoCabinet_FOUND)


  IF(Glog_FOUND)
    MESSAGE(STATUS "Google Glog:")
    MESSAGE(STATUS "  header: ${Glog_INCLUDE_DIRS}")
    MESSAGE(STATUS "  libs: ${Glog_LIBRARIES}")
  ELSE(Glog_FOUND)
    MESSAGE(STATUS "Google Glog: not found")
  ENDIF(Glog_FOUND)


  IF(Toku_FOUND)
    MESSAGE(STATUS "Toku_FOUND:")
    MESSAGE(STATUS "  header: ${Toku_INCLUDE_DIR}")
    MESSAGE(STATUS "  libs: ${Toku_LIBS}")
  ELSE(Toku_FOUND)
    MESSAGE(STATUS "Toku_FOUND: not found")
  ENDIF(Toku_FOUND)


  MESSAGE(STATUS "----------------------------------------")
  MESSAGE(STATUS "COMPONENTS BUILD:")
  FOREACH(c ${REAL_BUILD_COMPONENTS})
    MESSAGE(STATUS "  ${c}")
  ENDFOREACH(c ${REAL_BUILD_COMPONENTS})
  IF(REAL_NON_BUILD_COMPONENTS)
    MESSAGE(STATUS "COMPONENTS NOT BUILD:")
    FOREACH(c ${REAL_NON_BUILD_COMPONENTS})
      MESSAGE(STATUS "  ${c}")
    ENDFOREACH(c ${REAL_NON_BUILD_COMPONENTS})
  ENDIF(REAL_NON_BUILD_COMPONENTS)
  MESSAGE(STATUS "========================================")

ENDIF(VERBOSE)

CONFIGURE_FILE( "${CMAKE_SOURCE_DIR}/include/3rdparty/am/google/sparseconfig.h.in"
  "${CMAKE_SOURCE_DIR}/include/3rdparty/am/google/sparseconfig.h"
  ESCAPE_QUOTES @ONLY)

##################################################
# CPack
#####
INCLUDE(UseCPack)
CONFIGURE_FILE(
  "izenelib-version.h.cmake"
  "${CMAKE_SOURCE_DIR}/include/izenelib-version.h"
  @ONLY
  )
SET(CPACK_DEBIAN_PACKAGE_DEPENDS
  "libboost-dev, libbz2-dev, libz-dev, libc6-dev")
