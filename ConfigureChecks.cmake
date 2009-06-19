##################################################
# Load Modules
#####
INCLUDE(CheckIncludeFile)

##################################################
# Int Types
#####
CHECK_INCLUDE_FILE(inttypes.h HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE(stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILE(sys/types.h HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILE(sys/stat.h HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE(stddef.h HAVE_STDDEF_H)

##################################################
# Doxygen
#####
FIND_PACKAGE(Doxygen)
IF(DOXYGEN_DOT_EXECUTABLE)
  OPTION(USE_DOT "use dot in doxygen?" FLASE)
ENDIF(DOXYGEN_DOT_EXECUTABLE)

SET(USE_DOT_YESNO NO)
IF(USE_DOT)
  SET(USE_DOT_YESNO YES)
ENDIF(USE_DOT)

##################################################
# Boost
#####

# Only required header only library
# set BOOST_ROOT to select a boost installation

# This is required to enable the new version
SET(Boost_ADDITIONAL_VERSIONS 1.39 1.39.0 1.38 1.38.0 1.37 1.37.0)
# find unit_test_framework in boost >= 1.35, it's optional and only
# used in unit testcases
FIND_PACKAGE(Boost 1.35.0 REQUIRED)

##################################################
# Threads
#####
FIND_PACKAGE(Threads)

##################################################
# bzip2 & zlib
#####

FIND_PACKAGE(BZip2)
FIND_PACKAGE(ZLIB)

##################################################
# Tokoyo Cabnet
#####
FIND_PACKAGE(TokyoCabinet 1.4.24)
