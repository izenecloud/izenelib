FILE(GLOB RE2_SRC
	"*.cc"
)

MACRO_ADD_LIBRARY(re2
    SRCS
    ${RE2_SRC}
)



