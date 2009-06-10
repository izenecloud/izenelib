CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(IZENELIB)

# Find modules here before system modules dir
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

# force out-of-source build
IF (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
  MESSAGE(FATAL_ERROR "In source build is not allowed")
ENDIF (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)

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
IF (NOT BUILD_FLAGS_HAS_BEEN_CUSTOMIZED)

  # Set available types for build tool suppoting it, such as VS
  IF (NOT CMAKE_CONFIGURATION_TYPES)
    SET(CMAKE_CONFIGURATION_TYPES RelWithDebugInfo Release MinSizeRel Debug)
  ENDIF (NOT CMAKE_CONFIGURATION_TYPES)

  # Add two build type
  SET(CMAKE_CONFIGURATION_TYPES ${CMAKE_CONFIGURATION_TYPES} "Debugfull" "Profile" CACHE STRING "avalable build types" FORCE)

  # Customize For GNU/GCC
  IF (CMAKE_COMPILER_IS_GNUCXX)

    # Customize options for the build types
    SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g" CACHE STRING "Flags used by the compiler during Release with Debug Info builds." FORCE)
    SET(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG" CACHE STRING "Flags used by the compiler during release builds" FORCE)
    SET(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG" CACHE STRING "Flags used by the compiler during release minsize builds." FORCE)
    SET(CMAKE_CXX_FLAGS_DEBUG          "-g -O2 -fno-reorder-blocks -fno-schedule-insns -fno-inline" CACHE STRING "Flags used by the compiler during debug builds." FORCE)
    SET(CMAKE_CXX_FLAGS_DEBUGFULL      "-g3 -fno-inline" CACHE STRING "Debugfull build type" FORCE)
    SET(CMAKE_CXX_FLAGS_PROFILE        "-g3 -fno-inline -ftest-coverage -fprofile-arcs" CACHE STRING "Profile build type" FORCE)

  ENDIF (CMAKE_COMPILER_IS_GNUCXX)

  SET(BUILD_FLAGS_HAS_BEEN_CUSTOMIZED yes CACHE INTERNEL "build falgs has been customized?")
  MARK_AS_ADVANCED(CMAKE_CONFIGURATION_TYPES CMAKE_CXX_FLAGS_DEBUGFULL CMAKE_CXX_FLAGS_PROFILE)

ENDIF (NOT BUILD_FLAGS_HAS_BEEN_CUSTOMIZED)

# Set default build type
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE RelWithDebugInfo)
ENDIF(NOT CMAKE_BUILD_TYPE)
SET(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
    "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel Debugfull Profile." FORCE)

INCLUDE(SetCompilerWarningAll)
# Write to cache
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE STRING
  "Flags used by the compiler during all build types." FORCE)


##################################################
# Project Info
#####
SET(IZENELIB_VERSION_MAJOR 1)
SET(IZENELIB_VERSION_MINOR 0)
SET(IZENELIB_VERSION_PATCH 0)
SET(IZENELIB_VERSION "${IZENELIB_VERSION_MAJOR}.${IZENELIB_VERSION_MINOR}.${IZENELIB_VERSION_PATCH}")
SET(IZENELIB_PACKAGE "izenelib")
SET(IZENELIB_VERSION_STRING "${IZENELIB_PACKAGE} ${IZENELIB_VERSION}")

##################################################
# Check Dependencies
####
INCLUDE(SetPkgConfigPath)
INCLUDE(ConfigureChecks.cmake)

##################################################
# Messages
#####
MESSAGE(STATUS "========================================")
MESSAGE(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
STRING(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)
MESSAGE(STATUS "CORRESPONDING FLAGS: CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE_UPPER}")
MESSAGE(STATUS "")
MESSAGE(STATUS "CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG}")
MESSAGE(STATUS "CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE}")
MESSAGE(STATUS "CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
MESSAGE(STATUS "CMAKE_CXX_FLAGS_MINSIZEREL ${CMAKE_CXX_FLAGS_MINSIZEREL}")
MESSAGE(STATUS "CMAKE_CXX_FLAGS_DEBUGFULL ${CMAKE_CXX_FLAGS_DEBUGFULL}")
MESSAGE(STATUS "CMAKE_CXX_FLAGS_PROFILE ${CMAKE_CXX_FLAGS_PROFILE}")
MESSAGE(STATUS "----------------------------------------")
IF(Boost_FOUND)
  MESSAGE(STATUS "BOOST:")
  MESSAGE(STATUS "  header: ${Boost_INCLUDE_DIR}")
  MESSAGE(STATUS "  version: ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")
ELSE(Boost_FOUND)
  MESSAGE(STATUS "BOOST: not found")
ENDIF(Boost_FOUND)
IF(TokyoCabinet_FOUND)
  MESSAGE(STATUS "Tokyo Cabinet:")
  MESSAGE(STATUS "  header: ${TokyoCabinet_INCLUDE_DIR}")
  MESSAGE(STATUS "  libs: ${TokyoCabinet_LIBRARY}")
  MESSAGE(STATUS "  version: ${TokyoCabinet_VERSION}")
ELSE(TokyoCabinet_FOUND)
  MESSAGE(STATUS "TokyoCabinet: not found")
ENDIF(TokyoCabinet_FOUND)
MESSAGE(STATUS "========================================")
