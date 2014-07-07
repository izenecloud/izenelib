MACRO_CHECK_SHOULD_BUILD(para2vec)

IF(para2vec_SHOULD_BUILD)
  SET(para2vec_SRC
    word2vec.cpp
    paragraphvec.cpp)

  MACRO_ADD_LIBRARY(para2vec SRCS ${para2vec_SRC})

ENDIF(para2vec_SHOULD_BUILD)
