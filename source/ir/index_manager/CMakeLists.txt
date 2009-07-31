PROJECT(index_manager)

file(GLOB_RECURSE INDEX_MANAGER_SOURCE RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	"index/*.cpp"
	"store/*.cpp"
	"utility/*.cpp")

IF(NOT WISELIB_INCLUDE_DIR)
  MESSAGE(FATAL_ERROR
    "Cannot find WISELIB_INCLUDE_DIR.
Please set WISELIB_INCLUDE_DIR (-DWISELIB_INCLUDE_DIR=/path/to/wiselib)to path of the cloned repository. ")
ENDIF(NOT WISELIB_INCLUDE_DIR)

IF(IS_DIRECTORY "${WISELIB_INCLUDE_DIR}/include")
  SET(WISELIB_INCLUDE_DIR ${WISELIB_INCLUDE_DIR} "${WISELIB_INCLUDE_DIR}/include")
ENDIF(IS_DIRECTORY "${WISELIB_INCLUDE_DIR}/include")


FIND_PACKAGE(Boost REQUIRED)

set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)
find_package( Boost 1.36.0 COMPONENTS serialization filesystem system thread)


INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
INCLUDE_DIRECTORIES ( ${WISELIB_INCLUDE_DIR})


set( CXXFLAGS "-Wall -O3 -fPIC -DUDT3" )


SET_PROPERTY(GLOBAL APPEND PROPERTY BUILD_COMPONENTS index_manager)

SET(index_manager_SRC "${INDEX_MANAGER_SOURCE}")

ADD_LIBRARY(index_manager_shared SHARED ${index_manager_SRC})
SET_TARGET_PROPERTIES(index_manager_shared PROPERTIES
  OUTPUT_NAME "index_manager"
  COMPILE_FLAGS ${CXXFLAGS} 
  CLEAN_DIRECT_OUTPUT 1
  VERSION ${PROJECT_VERSION}
  )
TARGET_LINK_LIBRARIES(index_manager_shared  ${sdb_btree_shared}
#										${udt_shared}
										${streamflow_shared}
										${udt3_shared}
										)

  

ADD_LIBRARY(index_manager_static STATIC ${index_manager_SRC})
SET_TARGET_PROPERTIES(index_manager_static PROPERTIES
  OUTPUT_NAME "index_manager"
  COMPILE_FLAGS ${CXXFLAGS} 
  CLEAN_DIRECT_OUTPUT 1
  )
TARGET_LINK_LIBRARIES(index_manager_static  ${Boost_LIBRARIES}
										${sdb_btree_static}
#										${udt_static}
										${streamflow_static}
										${udt3_static}
										)

INSTALL(TARGETS index_manager_static index_manager_shared
  EXPORT index_manager
  ARCHIVE DESTINATION ${VERSION_INSTALL_PREFIX}/lib
  LIBRARY DESTINATION shared
  COMPONENT "index_manager"
  )

INSTALL(EXPORT index_manager
  DESTINATION ${VERSION_INSTALL_PREFIX}/cmake
  COMPONENT "index_manager"
  )


