PROJECT(index_manager)

file(GLOB_RECURSE INDEX_MANAGER_SOURCE RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	"index/*.cpp"
	"store/*.cpp"
	"utility/*.cpp")

FIND_PACKAGE(Wiselib REQUIRED)
FIND_PACKAGE(Boost REQUIRED)

set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)
find_package( Boost 1.36.0 COMPONENTS serialization filesystem system thread)


INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
INCLUDE_DIRECTORIES ( ${izenelib_SOURCE_DIR}/include/ )
INCLUDE_DIRECTORIES ( ${WISELIB_INCLUDE_DIR})


set(CPP "${INDEX_MANAGER_SOURCE}")
set( CXXFLAGS "-Wall -O3 -fPIC -DUDT3" )

set ( LIBRARY_OUTPUT_PATH ${izenelib_SOURCE_DIR}/lib ) 

LINK_DIRECTORIES( ${izenelib_SOURCE_DIR}/lib   $usr/local/lib ) 
    
ADD_LIBRARY(index_manager_shared SHARED ${CPP})
SET_TARGET_PROPERTIES(index_manager_shared 
				PROPERTIES 
				OUTPUT_NAME "index_manager" 
				COMPILE_FLAGS ${CXXFLAGS} 
				COMPILE_DEFINITIONS_DEBUG "-g" 
				)
TARGET_LINK_LIBRARIES(index_manager_shared ${WISELIB_LIBRARY}  
										sdb_btree
#										udt
										streamflow
										udt3
										)
SET_TARGET_PROPERTIES(index_manager_shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)



ADD_LIBRARY(index_manager_static STATIC ${CPP})
SET_TARGET_PROPERTIES(index_manager_static 
				PROPERTIES 
				OUTPUT_NAME "index_manager" 
				COMPILE_FLAGS ${CXXFLAGS} 
				COMPILE_DEFINITIONS_DEBUG "-g" 
				)
SET_TARGET_PROPERTIES(index_manager_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
TARGET_LINK_LIBRARIES(index_manager_static ${WISELIB_LIBRARY}
										${Boost_LIBRARIES}
										sdb_btree
#										udt
										streamflow
										udt3
										)


