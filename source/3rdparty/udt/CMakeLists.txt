PROJECT(udt)

file(GLOB_RECURSE UDT_SOURCE "*.cpp")

include_directories ( 
		${izenelib_SOURCE_DIR}/include/3rdparty/udt
		)
# This section build the libraries
set(CPP "${UDT_SOURCE}")

set( CXXFLAGS "-Wall -O3 -DAMD64" )
        
set ( LIBRARY_OUTPUT_PATH ${izenelib_SOURCE_DIR}/lib ) 

ADD_LIBRARY(udt_shared SHARED ${CPP})
SET_TARGET_PROPERTIES(udt_shared 
				PROPERTIES 
				OUTPUT_NAME "udt" 
				COMPILE_FLAGS ${CXXFLAGS} 
#COMPILE_DEFINITIONS_DEBUG "-g" 
				)
SET_TARGET_PROPERTIES(udt_shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)

ADD_LIBRARY(udt_static STATIC ${CPP})
SET_TARGET_PROPERTIES(udt_static 
				PROPERTIES 
				OUTPUT_NAME "udt" 
				COMPILE_FLAGS ${CXXFLAGS} 
#COMPILE_DEFINITIONS_DEBUG "-g" 
				)
SET_TARGET_PROPERTIES(udt_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)


