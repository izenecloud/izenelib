##################################################
# Load Modules
#####
INCLUDE(CheckIncludeFile)

##################################################
# Boost
#####

# Only required header only library
# set BOOST_ROOT to select a boost installation

# This is required to enable the new version
SET(Boost_ADDITIONAL_VERSIONS 1.37 1.37.0 1.38 1.38.0 1.39 1.39.0)
# find unit_test_framework in boost >= 1.35, it's optional and only
# used in unit testcases
FIND_PACKAGE(Boost 1.35.0 COMPONENTS unit_test_framework)
# require header only libraries >= 1.35
find_package(Boost 1.35.0 REQUIRED)

##################################################
# bzip2
#####

FIND_PACKAGE(BZip2)

##################################################
# Tokoyo Cabnet
#####
FIND_PACKAGE(TokyoCabinet 1.4.24)

##################################################
# Int Types
#####
###########
CHECK_INCLUDE_FILE(inttypes.h HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE(stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILE(sys/types.h HAVE_SYS_TYPES_H)
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
