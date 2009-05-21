PROJECT(boost_memory)

include_directories ( 
		${izenelib_SOURCE_DIR}/include
		${izenelib_SOURCE_DIR}/include/3rdparty
		)

file(GLOB_RECURSE BOOST_MEMORY_SOURCE "memory/*.cpp")

# This section build the libraries
SET(CPP "${BOOST_MEMORY_SOURCE}")

set ( LIBRARY_OUTPUT_PATH ${izenelib_SOURCE_DIR}/lib ) 

ADD_LIBRARY(boost_memory_shared SHARED ${CPP})
SET_TARGET_PROPERTIES(boost_memory_shared PROPERTIES OUTPUT_NAME "boost_memory")
SET_TARGET_PROPERTIES(boost_memory_shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)

ADD_LIBRARY(boost_memory_static STATIC ${CPP})
SET_TARGET_PROPERTIES(boost_memory_static PROPERTIES OUTPUT_NAME "boost_memory")
SET_TARGET_PROPERTIES(boost_memory_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

      
