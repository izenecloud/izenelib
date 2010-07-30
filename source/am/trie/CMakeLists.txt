MACRO_CHECK_SHOULD_BUILD(btrie)

IF(btrie_SHOULD_BUILD)

  SET(btrie_SRC alphabet_en.cpp alphabet_cjk.cpp)
  MACRO_ADD_LIBRARY(btrie SRCS ${btrie_SRC})

ENDIF(btrie_SHOULD_BUILD)