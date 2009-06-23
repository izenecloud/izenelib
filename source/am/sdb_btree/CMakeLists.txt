# also depends on febird, boost_thread, boost_serialization, but not
# required in building the library

IF(HAVE_SYS_STAT_H)

  SET_PROPERTY(GLOBAL APPEND PROPERTY BUILD_COMPONENTS sdb_btree)

  SET(sdb_btree_SRC sdb_btree_header.cpp)

  INCLUDE(MacroAddComponent)
  MACRO_ADD_COMPONENT(sdb_btree "${VERSION_INSTALL_PREFIX}" "${PROJECT_VERSION}" ${sdb_btree_SRC})

ELSE(HAVE_SYS_STAT_H)

  SET_PROPERTY(GLOBAL APPEND PROPERTY NON_BUILD_COMPONENTS sdb_btree)
  MESSAGE(STATUS "! WARNING: Cannot find <sys/stat.h> to build sdb_btree")

ENDIF(HAVE_SYS_STAT_H)