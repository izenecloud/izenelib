MACRO_CHECK_SHOULD_BUILD(llalloc)

IF(llalloc_SHOULD_BUILD)

  SET(llalloc_DEFINITIONS "-fomit-frame-pointer -Wcast-qual -Wmissing-format-attribute -Wstrict-aliasing 
  -Wsign-compare -Wdeclaration-after-statement -Wnested-externs -Wdisabled-optimization -Winline -Wundef 
  -Wimplicit -Wunused -Wfloat-equal -Winit-self -Wformat=2 -Wswitch -Wsequence-point -Wparentheses -Wimplicit 
  -Wchar-subscripts -Wredundant-decls -Wstrict-prototypes -Wbad-function-cast -Wpointer-arith -Wwrite-strings 
  -Wno-long-long -Wmissing-declarations -Wmissing-prototypes -Wextra -Wall -pedantic -ggdb3 -std=gnu99 
  -O3 -fPIC -DPIC -c -D_REENTRANT")

  ADD_DEFINITIONS("${llalloc_DEFINITIONS}")
  MESSAGE(STATUS "llalloc_DEFINITIONS=${llalloc_DEFINITIONS}")

  MACRO_ADD_LIBRARY(llalloc SRCS "ll_alloc.c")

  TARGET_LINK_LIBRARIES(llalloc pthread)

ENDIF(llalloc_SHOULD_BUILD)
