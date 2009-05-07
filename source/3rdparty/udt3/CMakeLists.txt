PROJECT(udt3)

file(GLOB_RECURSE UDT3_SOURCE "*.cpp")

include_directories ( 
		${izenelib_SOURCE_DIR}/include/3rdparty/udt3
		)
# This section build the libraries
set(CPP "${UDT3_SOURCE}")

set( CXXFLAGS "-fPIC -Wall -finline-functions -O3 -DIA32 -DLINUX -DNO_BUSY_WAITING" )
        
set ( LIBRARY_OUTPUT_PATH ${izenelib_SOURCE_DIR}/lib ) 

ADD_LIBRARY(udt3_shared SHARED ${CPP})
SET_TARGET_PROPERTIES(udt3_shared 
				PROPERTIES 
				OUTPUT_NAME "udt3" 
				COMPILE_FLAGS ${CXXFLAGS} 
#COMPILE_DEFINITIONS_DEBUG "-g" 
				)
SET_TARGET_PROPERTIES(udt3_shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)

ADD_LIBRARY(udt3_static STATIC ${CPP})
SET_TARGET_PROPERTIES(udt3_static 
				PROPERTIES 
				OUTPUT_NAME "udt3" 
				COMPILE_FLAGS ${CXXFLAGS} 
#COMPILE_DEFINITIONS_DEBUG "-g" 
				)
SET_TARGET_PROPERTIES(udt3_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)


