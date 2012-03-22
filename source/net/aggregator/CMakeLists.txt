MACRO_CHECK_SHOULD_BUILD(
  aggregator
  Boost_FOUND
  Glog_FOUND
  )

IF(aggregator_SHOULD_BUILD)

  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${Glog_INCLUDE_DIRS}
    )

  FILE(GLOB
    aggregator_SRC
    "*.cpp"
    )
  ADD_DEFINITIONS("-fPIC -Wno-sign-compare")
  MACRO_ADD_LIBRARY(aggregator SRCS ${aggregator_SRC})
  
  TARGET_LINK_LIBRARIES(aggregator
      izene_util
    )
ENDIF(aggregator_SHOULD_BUILD)
