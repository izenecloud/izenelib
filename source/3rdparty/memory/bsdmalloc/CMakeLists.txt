MACRO_CHECK_SHOULD_BUILD(bsdmalloc)

IF(bsdmalloc_SHOULD_BUILD)

  MACRO_ADD_LIBRARY(bsdmalloc
    SRCS "OpenBSD_malloc_Linux.c"
    )

ENDIF(bsdmalloc_SHOULD_BUILD)
