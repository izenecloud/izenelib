FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS serialization filesystem system thread)

MACRO_CHECK_SHOULD_BUILD(
  sf1common 
  Glog_FOUND
  Boost_FOUND
  )

IF(sf1common_SHOULD_BUILD)

  FILE(GLOB_RECURSE
    sf1common_SRC
    RELATIVE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "*.cpp"
    )

  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${IZENELIB_SOURCE_DIR}/include/3rdparty/am
    ${Glog_INCLUDE_DIRS}
    )

  ADD_DEFINITIONS("-fPIC -DUDT3")

  MACRO_ADD_LIBRARY(sf1common SRCS ${sf1common_SRC})

ENDIF(sf1common_SHOULD_BUILD)

