MACRO_CHECK_SHOULD_BUILD(
  distribute
  Boost_FOUND
  Glog_FOUND
  )

IF(distribute_SHOULD_BUILD)

  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${Glog_INCLUDE_DIRS}
    )

  FILE(GLOB
    distribute_SRC
    "*.cpp"
    )
  ADD_DEFINITIONS("-fPIC -Wno-sign-compare")
  MACRO_ADD_LIBRARY(distribute SRCS ${distribute_SRC})
  
  TARGET_LINK_LIBRARIES(distribute
      izene_util
    )
ENDIF(distribute_SHOULD_BUILD)
