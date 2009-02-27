PROJECT(sdb_hash)

# This section build the librares
SET(CPP bucket_chain.cpp)
        
SET(LIBRARY_OUTPUT_PATH ../../../lib)

INCLUDE_DIRECTORIES(../../../include/am/sdb_hash)

ADD_LIBRARY(sdb_hash_shared SHARED ${CPP})
SET_TARGET_PROPERTIES(sdb_hash_shared PROPERTIES OUTPUT_NAME "sdb_hash")
SET_TARGET_PROPERTIES(sdb_hash_shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)

ADD_LIBRARY(sdb_hash_static STATIC ${CPP})
SET_TARGET_PROPERTIES(sdb_hash_static PROPERTIES OUTPUT_NAME "sdb_hash")
SET_TARGET_PROPERTIES(sdb_hash_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)


