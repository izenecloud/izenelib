PROJECT(ProcMemInfo)

include_directories ( 
		../../include/util
		)
# This section build the libraries
SET(CPP ProcMemInfo.cpp)
        
SET(LIBRARY_OUTPUT_PATH ../../../lib)

ADD_LIBRARY(ProcMemInfo_shared SHARED ${CPP})
SET_TARGET_PROPERTIES(ProcMemInfo_shared PROPERTIES OUTPUT_NAME "ProcMemInfo")
SET_TARGET_PROPERTIES(ProcMemInfo_shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)

ADD_LIBRARY(ProcMemInfo_static STATIC ${CPP})
SET_TARGET_PROPERTIES(ProcMemInfo_static PROPERTIES OUTPUT_NAME "ProcMemInfo")
SET_TARGET_PROPERTIES(ProcMemInfo_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)


