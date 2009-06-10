# - Enable warning all for gcc or use /W4 for visual studio

#
# TODO:
# - Make it more portable

## Strict warning level
IF (CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake)")
  # Use the highest warning level for visual studio.
  SET(CMAKE_CXX_WARNING_LEVEL 4)
  IF (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    STRING(REGEX REPLACE "/W[0-4]" "/W4"
      CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  ELSE (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  ENDIF (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
ELSEIF (CMAKE_COMPILER_IS_GNUCXX)
  # use -Wall for gcc
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
ELSE (CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake)")
  MESSAGE(STATUS "Unknown build tool, cannot set warning flags for your")
ENDIF (CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake)")
