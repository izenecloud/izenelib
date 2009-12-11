MACRO_CHECK_SHOULD_BUILD(boost_memory)

IF(boost_memory_SHOULD_BUILD)

  SET(boost_memory_SRC memory/memory.cpp)
  ADD_LIBRARY(boost_memory ${boost_memory_SRC})
  #INSTALL(TARGETS boost_memory DESTINATION lib)
  INSTALL(TARGETS boost_memory        
      LIBRARY DESTINATION lib 
      COMPONENT "izenelib_libraries")

ENDIF(boost_memory_SHOULD_BUILD)
