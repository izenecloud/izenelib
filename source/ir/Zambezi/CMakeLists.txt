MACRO_CHECK_SHOULD_BUILD(
  zambezi
  Boost_FOUND
  )

IF(zambezi_SHOULD_BUILD)

  FILE(GLOB_RECURSE
    zambezi_SRC
    RELATIVE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "*.cpp"
    )

  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${Glog_INCLUDE_DIRS}
    )

  MACRO_ADD_LIBRARY(zambezi SRCS ${zambezi_SRC})

ENDIF(zambezi_SHOULD_BUILD)
