PROJECT(streamflow)

file(GLOB_RECURSE STREAMFLOW_SOURCE RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	"streamflow.c"
	"override.c"
	"malloc_new.cpp")


INCLUDE_DIRECTORIES ( ${izenelib_SOURCE_DIR}/include/3rdparty/streamflow )
INCLUDE_DIRECTORIES ( ${izenelib_SOURCE_DIR}/include/3rdparty/streamflow/include-x86_64 )

set(CPP "${STREAMFLOW_SOURCE}")
set( CXXFLAGS "-O3 -ggdb -D_GNU_SOURCE -Wall -m64 -fno-strict-aliasing -fPIC -D_REENTRANT -Dx86_64 -DRADIX_TREE" )
#set( CXXFLAGS "-O3 -ggdb -D_GNU_SOURCE -Wall -m32 -fno-strict-aliasing -D_REENTRANT -Dx86 -DRADIX_TREE" )
#set( CXXFLAGS "-O3 -ggdb -D_GNU_SOURCE -Wall -fno-strict-aliasing -fPIC -D_REENTRANT -Dia64 -DRADIX_TREE" )

set ( LIBRARY_OUTPUT_PATH ${izenelib_SOURCE_DIR}/lib ) 

LINK_DIRECTORIES( ${izenelib_SOURCE_DIR}/lib) 
    
ADD_LIBRARY(streamflow_shared SHARED ${CPP})
SET_TARGET_PROPERTIES(streamflow_shared 
				PROPERTIES 
				OUTPUT_NAME "streamflow" 
				COMPILE_FLAGS ${CXXFLAGS} 
				)

SET_TARGET_PROPERTIES(streamflow_shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)

TARGET_LINK_LIBRARIES(streamflow_shared pthread m dl)

