##################################################
# Load Modules
#####
INCLUDE(CheckIncludeFile)
INCLUDE(CheckIncludeFileCXX)
##################################################
# Int Types
#####

CHECK_INCLUDE_FILE(inttypes.h HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE(stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILE(sys/types.h HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILE(sys/stat.h HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE(sys/timerfd.h HAVE_SYS_TIMERFD_H)
CHECK_INCLUDE_FILE(sys/signalfd.h HAVE_SYS_SIGNALFD_H)
CHECK_INCLUDE_FILE(sys/socket.h HAVE_SYS_SOCKET_H)
CHECK_INCLUDE_FILE(sys/time.h HAVE_SYS_TIME_H)
CHECK_INCLUDE_FILE(sys/types.h HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILE(sys/utsname.h HAVE_SYS_UTSNAME_H)
CHECK_INCLUDE_FILE(arpa/inet.h HAVE_ARPA_INET_H)
CHECK_INCLUDE_FILE(dlfcn.h HAVE_DLFCN_H)
CHECK_INCLUDE_FILE(fcntl.h HAVE_FCNTL_H)
CHECK_INCLUDE_FILE(memory.h HAVE_MEMORY_H)
CHECK_INCLUDE_FILE(netdb.h HAVE_NETDB_H)
CHECK_INCLUDE_FILE(netdb/in.h HAVE_NETINET_IN_H)
CHECK_INCLUDE_FILE(stdlib.h HAVE_STDLIB_H)
CHECK_INCLUDE_FILE(strings.h HAVE_STRINGS_H)
CHECK_INCLUDE_FILE(string.h HAVE_STRING_H)
CHECK_INCLUDE_FILE(unistd.h HAVE_UNISTD_H)
CHECK_INCLUDE_FILE(stddef.h HAVE_STDDEF_H)
CHECK_INCLUDE_FILE_CXX(ext/hash_fun.h HAVE_EXT_HASH_FUN_H)
CHECK_INCLUDE_FILE_CXX(backward/hash_fun.h HAVE_BACKWARD_HASH_FUN_H)

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
SET(Boost_ADDITIONAL_VERSIONS 1.47 1.47.0)
FIND_PACKAGE(Boost 1.47 REQUIRED
  COMPONENTS
  system
  program_options
  thread
  regex
  date_time
  serialization
  filesystem
  unit_test_framework
  iostreams
  )

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

##################################################
# Glog
#####
FIND_PACKAGE(Glog)

##################################################
# Thrift
#####
FIND_PACKAGE(Thrift)

##################################################
# Tokudb
#####
FIND_PACKAGE(Toku)
##################################################
# Other common libraries
#####

FIND_PACKAGE(OpenSSL)

FIND_LIBRARY(DL_LIBRARIES NAMES dl)
FIND_LIBRARY(M_LIBRARIES NAMES m)
