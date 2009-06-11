PROJECT(iconv)

file(GLOB_RECURSE ICONV_SOURCE "*.c")

include_directories ( 
		${izenelib_SOURCE_DIR}/include/3rdparty/libiconv-1.13/include/
		)
# This section build the libraries
set(CPP "${ICONV_SOURCE}")

set( CXXFLAGS "-fPIC -Wall -finline-functions -O3 -DIA32 -DLINUX -DNO_BUSY_WAITING" )
        
set ( LIBRARY_OUTPUT_PATH ${izenelib_SOURCE_DIR}/lib ) 

ADD_LIBRARY(iconv_shared SHARED ${CPP})
SET_TARGET_PROPERTIES(iconv_shared 
				PROPERTIES 
				OUTPUT_NAME "iconv" 
				COMPILE_FLAGS ${CXXFLAGS} 
#COMPILE_DEFINITIONS_DEBUG "-g" 
				)
SET_TARGET_PROPERTIES(iconv_shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)

ADD_LIBRARY(iconv_static STATIC ${CPP})
SET_TARGET_PROPERTIES(iconv_static 
				PROPERTIES 
				OUTPUT_NAME "iconv" 
				COMPILE_FLAGS ${CXXFLAGS} 
#COMPILE_DEFINITIONS_DEBUG "-g" 
				)
SET_TARGET_PROPERTIES(iconv_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)


