MACRO_CHECK_SHOULD_BUILD(btrie)

IF(btrie_SHOULD_BUILD)

  SET(btrie_SRC alphabet_en.cpp alphabet_cjk.cpp)
  ADD_LIBRARY(btrie ${btrie_SRC})

ENDIF(btrie_SHOULD_BUILD)