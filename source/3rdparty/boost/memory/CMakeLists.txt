SET_PROPERTY(GLOBAL APPEND PROPERTY BUILD_COMPONENTS boost_memory)

SET(boost_memory_SRC memory.cpp)

ADD_LIBRARY(boost_memory_shared SHARED ${boost_memory_SRC})
SET_TARGET_PROPERTIES(boost_memory_shared PROPERTIES
  OUTPUT_NAME "boost_memory"
  CLEAN_DIRECT_OUTPUT 1
  VERSION ${IZENELIB_VERSION}
  )

ADD_LIBRARY(boost_memory_static STATIC ${boost_memory_SRC})
SET_TARGET_PROPERTIES(boost_memory_static PROPERTIES
  OUTPUT_NAME "boost_memory"
  CLEAN_DIRECT_OUTPUT 1
  )

INSTALL(TARGETS boost_memory_static boost_memory_shared
  EXPORT boost_memory
  ARCHIVE DESTINATION ${VERSION_INSTALL_PREFIX}/lib
  LIBRARY DESTINATION shared)

INSTALL(EXPORT boost_memory
  DESTINATION ${VERSION_INSTALL_PREFIX}/cmake)
