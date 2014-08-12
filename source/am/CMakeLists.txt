FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS serialization filesystem system thread)

MACRO_CHECK_SHOULD_BUILD(
  am
  Glog_FOUND
  Boost_FOUND
  )

IF(am_SHOULD_BUILD)

  FILE(GLOB_RECURSE
    am_SRC
    RELATIVE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "filemapper/*.cpp"
    "vsynonym/*.cpp"
    "trie/*.cpp"
    "succinct/*.cpp"
    "succinct/*.cc"
    "succinct/*.c"
    "approximate_matching/*.cpp"
    "bitmap/*.cpp"
    )

  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${IZENELIB_SOURCE_DIR}/include/3rdparty/am
    ${Glog_INCLUDE_DIRS}
    )

  ADD_DEFINITIONS("-fPIC -DUDT3")

  MACRO_ADD_LIBRARY(am SRCS ${am_SRC})

ENDIF(am_SHOULD_BUILD)

MACRO_CHECK_SHOULD_BUILD(filemapper)
IF(filemapper_SHOULD_BUILD)
  MACRO_ADD_LIBRARY(filemapper SRCS filemapper/persist.cpp)
ENDIF(filemapper_SHOULD_BUILD)
