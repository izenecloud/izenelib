CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

##################################################
# !!! IMPORTANT
# Must at the beginning of the file and before PROJECT.
# 
# Set default install prefix for user, use following order to select
# the value:
#
# 1) Specified in cmake -DCMAKE_INSTALL_PREFIX=/your/prefix
# 2) Evironement variable MY_INSTALL_PREFIX
# 3) Try $HOME/myroot
#
# User also can change CMAKE_INSTALL_PREFIX in cache
#####
IF(NOT CMAKE_INSTALL_PREFIX)
  IF(IS_DIRECTORY "$ENV{MY_INSTALL_PREFIX}")
    SET(CMAKE_INSTALL_PREFIX "$ENV{MY_INSTALL_PREFIX}" CACHE STRING "Install path prefix, prepended onto install directories.")
  ELSEIF(IS_DIRECTORY "$ENV{HOME}/myroot")
    SET(CMAKE_INSTALL_PREFIX "$ENV{HOME}/myroot" CACHE STRING "Install path prefix, prepended onto install directories.")
  ENDIF(IS_DIRECTORY "$ENV{MY_INSTALL_PREFIX}")
ENDIF(NOT CMAKE_INSTALL_PREFIX)

##################################################
# Find extra cmake modules dirs
#
# You can clone all modules from izenesoft.com:/git/cmake.git.
# 
# Copy it to your install dir or change the variable
# EXTRA_CMAKE_MODULES_DIRS.
#
IF(IS_DIRECTORY "${CMAKE_INSTALL_PREFIX}/cmake/modules")
  SET(EXTRA_CMAKE_MODULES_DIRS ${EXTRA_CMAKE_MODULES_DIRS} "${CMAKE_INSTALL_PREFIX}/cmake/modules")
ENDIF(IS_DIRECTORY "${CMAKE_INSTALL_PREFIX}/cmake/modules")
IF(NOT EXTRA_CMAKE_MODULES_DIRS)
  MESSAGE(FATAL_ERROR
    "Cannot find EXTRA_CMAKE_MODULES_DIRS.
You should clone it from izenesoft.com:/git/cmake.git. Then set EXTRA_CMAKE_MODULES_DIRS (-DEXTRA_CMAKE_MODULES_DIRS=/path/to/cmake/modules) to path of subdirectory moduels in your cloned repository. You also can copy the cloned cmake directory to ${CMAKE_INSTALL_PREFIX}")
ENDIF(NOT EXTRA_CMAKE_MODULES_DIRS)
SET(CMAKE_MODULE_PATH ${EXTRA_CMAKE_MODULES_DIRS})

# force out-of-source build
IF (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
  MESSAGE(FATAL_ERROR "In source build is not allowed")
ENDIF (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)

PROJECT(IZENELIB)
OPTION(VERBOSE "print detailed configure messages" ON)

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
    SET(CMAKE_CONFIGURATION_TYPES RelWithDebInfo Release MinSizeRel Debug)
  ENDIF (NOT CMAKE_CONFIGURATION_TYPES)

  # Add two build type
  SET(CMAKE_CONFIGURATION_TYPES ${CMAKE_CONFIGURATION_TYPES} "Debugfull" "Profile" CACHE STRING "avalable build types" FORCE)

  # Customize For GNU/GCC
  IF (CMAKE_COMPILER_IS_GNUCXX)

    # Customize options for the build types
    SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g" CACHE STRING "Flags used by the compiler during Release with Debug Info builds." FORCE)
    SET(CMAKE_CXX_FLAGS_RELEASE          "-O3 -DNDEBUG" CACHE STRING "Flags used by the compiler during release builds" FORCE)
    SET(CMAKE_CXX_FLAGS_MINSIZEREL       "-Os -DNDEBUG" CACHE STRING "Flags used by the compiler during release minsize builds." FORCE)
    SET(CMAKE_CXX_FLAGS_DEBUG            "-g -O2 -fno-reorder-blocks -fno-schedule-insns -fno-inline" CACHE STRING "Flags used by the compiler during debug builds." FORCE)
    SET(CMAKE_CXX_FLAGS_DEBUGFULL        "-g3 -fno-inline" CACHE STRING "Debugfull build type" FORCE)
    SET(CMAKE_CXX_FLAGS_PROFILE          "-g3 -fno-inline -ftest-coverage -fprofile-arcs" CACHE STRING "Profile build type" FORCE)

  ENDIF (CMAKE_COMPILER_IS_GNUCXX)

  SET(BUILD_FLAGS_HAS_BEEN_CUSTOMIZED yes CACHE INTERNEL "build falgs has been customized?")
  MARK_AS_ADVANCED(CMAKE_CONFIGURATION_TYPES CMAKE_CXX_FLAGS_DEBUGFULL CMAKE_CXX_FLAGS_PROFILE)

ENDIF (NOT BUILD_FLAGS_HAS_BEEN_CUSTOMIZED)

# Set default build type
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE RelWithDebInfo)
ENDIF(NOT CMAKE_BUILD_TYPE)
SET(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
    "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel Debugfull Profile." FORCE)

INCLUDE(SetCompilerWarningAll)
# Write to cache
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE STRING
  "Flags used by the compiler during all build types." FORCE)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING
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
# Sub Directories
#####

# all headers
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/include")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/include/3rdparty")

# log components
SET_PROPERTY(GLOBAL PROPERTY BUILD_COMPONENTS)
SET_PROPERTY(GLOBAL PROPERTY NON_BUILD_COMPONENTS)

# prefix of bin include and static lib
SET(VERSION_INSTALL_PREFIX "izenelib-${IZENELIB_VERSION}")

# third party libraries
ADD_SUBDIRECTORY(source/3rdparty 3rdparty)

# # AM libraries
# ADD_SUBDIRECTORY(source/am am)

# IR libraries
ADD_SUBDIRECTORY(source/ir ir)

# Util libraries
ADD_SUBDIRECTORY(source/util util)

# Test cases and applications
# ADD_SUBDIRECTORY(source/test test)

GET_PROPERTY(BUILD_COMPONENTS GLOBAL PROPERTY BUILD_COMPONENTS)
GET_PROPERTY(NON_BUILD_COMPONENTS GLOBAL PROPERTY NON_BUILD_COMPONENTS)

##################################################
# Messages
#####
IF(NOT VERBOSE)
  RETURN()
ENDIF(NOT VERBOSE)
  
MESSAGE(STATUS "========================================")
MESSAGE(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
STRING(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)
MESSAGE(STATUS "  corresponding flags: ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE_UPPER}}")
MESSAGE(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
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
IF(TokyoCabinet_FOUND)
  MESSAGE(STATUS "Tokyo Cabinet:")
  MESSAGE(STATUS "  header: ${TokyoCabinet_INCLUDE_DIR}")
  MESSAGE(STATUS "  libs: ${TokyoCabinet_LIBRARY}")
  MESSAGE(STATUS "  version: ${TokyoCabinet_VERSION}")
ELSE(TokyoCabinet_FOUND)
  MESSAGE(STATUS "TokyoCabinet: not found")
ENDIF(TokyoCabinet_FOUND)
MESSAGE(STATUS "----------------------------------------")
MESSAGE(STATUS "COMPONENTS BUILD:")
FOREACH(c ${BUILD_COMPONENTS})
  MESSAGE(STATUS "  ${c}")
ENDFOREACH(c ${BUILD_COMPONENTS})
IF(NON_BUILD_COMPONENTS)
  MESSAGE(STATUS "COMPONENTS NOT BUILD:")
  FOREACH(c ${NON_BUILD_COMPONENTS})
    MESSAGE(STATUS "  ${c}")
  ENDFOREACH(c ${NON_BUILD_COMPONENTS})
ENDIF(NON_BUILD_COMPONENTS)
MESSAGE(STATUS "========================================")
