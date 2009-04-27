PROJECT(ProcMemInfo)

include_directories ( 
		${izenelib_SOURCE_DIR}/include/am/sdb_btree
		)
# This section build the libraries
SET(CPP sdb_btree_header.cpp)
        
set ( LIBRARY_OUTPUT_PATH ${izenelib_SOURCE_DIR}/lib ) 

ADD_LIBRARY( sdb_btree_shared SHARED ${CPP})
SET_TARGET_PROPERTIES( sdb_btree_shared PROPERTIES OUTPUT_NAME "sdb_btree")
SET_TARGET_PROPERTIES( sdb_btree_shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)

ADD_LIBRARY( sdb_btree_static STATIC ${CPP})
SET_TARGET_PROPERTIES( sdb_btree_static PROPERTIES OUTPUT_NAME "sdb_btree")
SET_TARGET_PROPERTIES( sdb_btree_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)


