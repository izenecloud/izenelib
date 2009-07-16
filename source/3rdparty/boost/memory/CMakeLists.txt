SET_PROPERTY(GLOBAL APPEND PROPERTY BUILD_COMPONENTS boost_memory)

SET(boost_memory_SRC memory.cpp)

MACRO_ADD_COMPONENT(boost_memory
  "${VERSION_INSTALL_PREFIX}"
  "${PROJECT_VERSION}"
  ${boost_memory_SRC}
  )
