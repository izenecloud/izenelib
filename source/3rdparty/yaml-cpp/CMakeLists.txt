MACRO_CHECK_SHOULD_BUILD(yaml)

IF(yaml_SHOULD_BUILD)
  FILE(GLOB yaml_SRC
	"*.cpp"
        "contrib/*.cpp"
)


  MACRO_ADD_LIBRARY(yaml SRCS ${yaml_SRC})

ENDIF(yaml_SHOULD_BUILD)
