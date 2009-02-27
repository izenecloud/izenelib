PROJECT(boost_memory)

include_directories ( 
		../../../../include/3rdparty
		)
# This section build the libraries
SET(CPP memory.cpp)
        
SET(LIBRARY_OUTPUT_PATH ../../../../lib)

ADD_LIBRARY(boost_memory_shared SHARED ${CPP})
SET_TARGET_PROPERTIES(boost_memory_shared PROPERTIES OUTPUT_NAME "boost_memory")
SET_TARGET_PROPERTIES(boost_memory_shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)

ADD_LIBRARY(boost_memory_static STATIC ${CPP})
SET_TARGET_PROPERTIES(boost_memory_static PROPERTIES OUTPUT_NAME "boost_memory")
SET_TARGET_PROPERTIES(boost_memory_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)


