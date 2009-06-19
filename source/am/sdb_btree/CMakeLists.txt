# also depends on febird, boost_thread, boost_serialization, but not
# required in building the library

IF(HAVE_SYS_STAT_H)

  SET_PROPERTY(GLOBAL APPEND PROPERTY BUILD_COMPONENTS sdb_btree)

  SET(sdb_btree_SRC sdb_btree_header.cpp)

  ADD_LIBRARY(sdb_btree_shared SHARED ${sdb_btree_SRC})
  SET_TARGET_PROPERTIES(sdb_btree_shared PROPERTIES
    OUTPUT_NAME "sdb_btree"
    CLEAN_DIRECT_OUTPUT 1
    VERSION ${PROJECT_VERSION}
    )

  ADD_LIBRARY(sdb_btree_static STATIC ${sdb_btree_SRC})
  SET_TARGET_PROPERTIES(sdb_btree_static PROPERTIES
    OUTPUT_NAME "sdb_btree"
    CLEAN_DIRECT_OUTPUT 1
    )

  INSTALL(TARGETS sdb_btree_static sdb_btree_shared
    EXPORT sdb_btree
    ARCHIVE DESTINATION ${VERSION_INSTALL_PREFIX}/lib
    LIBRARY DESTINATION shared
    COMPONENT "sdb_btree"
    )

  INSTALL(EXPORT sdb_btree
    DESTINATION ${VERSION_INSTALL_PREFIX}/cmake
    COMPONENT "sdb_btree"
    )

ELSE(HAVE_SYS_STAT_H)

  SET_PROPERTY(GLOBAL APPEND PROPERTY NON_BUILD_COMPONENTS sdb_btree)
  MESSAGE(STATUS "! WARNING: Cannot find <sys/stat.h> to build sdb_btree")

ENDIF(HAVE_SYS_STAT_H)