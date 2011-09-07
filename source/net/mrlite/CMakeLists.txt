MACRO_CHECK_SHOULD_BUILD(
  mrlite
  Boost_FOUND
  Glog_FOUND
  )

IF(mrlite_SHOULD_BUILD)

  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${Glog_INCLUDE_DIRS}
    )

  FILE(GLOB
    mrlite_SRC
    RELATIVE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "*.cpp"
    )
  ADD_DEFINITIONS("-fPIC -Wno-sign-compare")
  MACRO_ADD_LIBRARY(mrlite SRCS ${mrlite_SRC})

ENDIF(mrlite_SHOULD_BUILD)
