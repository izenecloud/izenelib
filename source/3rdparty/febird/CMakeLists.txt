PROJECT(febird)

file(GLOB_RECURSE FEBIRD_SOURCE "io/*.cpp")

include_directories ( 
		${izenelib_SOURCE_DIR}/include/3rdparty/febird/io
		)
# This section build the libraries
set(CPP "${FEBIRD_SOURCE}")
set( CXXFLAGS "-Wall -O3 -ext" )

set ( LIBRARY_OUTPUT_PATH ${izenelib_SOURCE_DIR}/lib ) 

ADD_LIBRARY(febird_shared SHARED ${CPP})
SET_TARGET_PROPERTIES(febird_shared 
				PROPERTIES 
				OUTPUT_NAME "febird" 
				COMPILE_FLAGS ${CXXFLAGS} 
#COMPILE_DEFINITIONS_DEBUG "-g" 
				)

SET_TARGET_PROPERTIES(febird_shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)

ADD_LIBRARY(febird_static STATIC ${CPP})
SET_TARGET_PROPERTIES(febird_static 
				PROPERTIES 
				OUTPUT_NAME "febird" 
				COMPILE_FLAGS ${CXXFLAGS} 
#COMPILE_DEFINITIONS_DEBUG "-g" 
				)
SET_TARGET_PROPERTIES(febird_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)


