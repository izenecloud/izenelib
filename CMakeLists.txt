project (izenelib)

cmake_minimum_required (VERSION 2.6)

set (IZENELIB_VERSION_MAJOR 1)
set (IZENELIB_VERSION_MINOR 0)
set (IZENELIB_VERSION_PATCH 0)
set (IZENELIB_VERSION "${IZENELIB_VERSION_MAJOR}.${IZENELIB_VERSION_MINOR}.${IZENELIB_VERSION_PATCH}")
set (IZENELIB_PACKAGE "izenelib")
set (IZENELIB_VERSION_STRING "${IZENELIB_PACKAGE} ${IZENELIB_VERSION}")

# force out-of-source build
if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
  message (FATAL_ERROR "In source build is not allowed")
endif (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)

####################
# set build type (set of compiler options)
#####

# default build type
if (NOT CMAKE_BUILD_TYPE) 
  set (CMAKE_BUILD_TYPE RelWithDebInfo)
endif (NOT CMAKE_BUILD_TYPE)

if (CMAKE_COMPILER_IS_GNUCXX)
  set (CMAKE_CONFIGURATION_TYPES ${CMAKE_CONFIGURATION_TYPES} "Debugfull" "Profile")

  # Select flags.
  set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
  set (CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG")
  set (CMAKE_CXX_FLAGS_DEBUG          "-g -O2 -fno-reorder-blocks -fno-schedule-insns -fno-inline")
  set (CMAKE_CXX_FLAGS_DEBUGFULL      "-g3 -fno-inline" CACHE STRING "Debugfull build type")
  set (CMAKE_CXX_FLAGS_PROFILE        "-g3 -fno-inline -ftest-coverage -fprofile-arcs" CACHE STRING "Profile build type")

  mark_as_advanced (CMAKE_CXX_FLAGS_DEBUGFULL CMAKE_CXX_FLAGS_PROFILE)
endif (CMAKE_COMPILER_IS_GNUCXX)

####################
# wanning wall
#####

if (CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake)")
  # Use the highest warning level for visual studio.
  set (CMAKE_CXX_WARNING_LEVEL 4)
  if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string (REGEX REPLACE "/W[0-4]" "/W4"
      CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
elseif (CMAKE_COMPILER_IS_GNUCXX)
  # use -Wall for gcc
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
else (CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake)")
  message (STATUS "Unknown build tool, cannot set warning flags for your")
endif (CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake)")

####################
# libraries
#####

# set BOOST_ROOT to select a boost installation
set (Boost_ADDITIONAL_VERSIONS 1.37 1.37.0 1.38 1.38.0 1.39 1.39.0)
# find libraries
find_package (Boost 1.35.0 COMPONENTS unit_test_framework)
# require header only libraries
find_package (Boost 1.35.0 REQUIRED)

####################
# utilites
#####

find_package (Doxygen)
if (DOXYGEN_DOT_EXECUTABLE)
  option (USE_DOT "use dot in doxygen?" FLASE)
endif (DOXYGEN_DOT_EXECUTABLE)

set (USE_DOT_YESNO NO)
if (USE_DOT)
  set (USE_DOT_YESNO YES)
endif (USE_DOT)

####################
# Doxygen Docs
#####
configure_file (
  "${CMAKE_SOURCE_DIR}/source/doxygen.cf.in"
  "${CMAKE_BINARY_DIR}/doxygen.cf"
  ESCAPE_QUOTES @ONLY)

if (DOXYGEN_FOUND)
  add_custom_target (docs ${DOXYGEN_EXECUTABLE} "doxygen.cf")
  add_custom_target (package_docs
    "tar" "-C" "docs" "-czf" "izenelib-${CLUSTERING_FRAMEWORK_VERSION}.tar.gz" "izenelib-doc"
    COMMAND "cp" "clustering-framework-${CLUSTERING_FRAMEWORK_VERSION}.tar.gz" "${CMAKE_SOURCE_DIR}/docs/html"
    DEPENDS docs
    )
  set_directory_properties (PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES docs)
endif (DOXYGEN_FOUND)

####################
# sub dirs
#####

add_subdirectory (source)
