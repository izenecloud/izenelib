MACRO_CHECK_SHOULD_BUILD(boost_memory)

IF(boost_memory_SHOULD_BUILD)

  MACRO_ADD_LIBRARY(boost_memory
    SRCS
    memory/memory.cpp
    )

ENDIF(boost_memory_SHOULD_BUILD)
