FIND_PACKAGE(Boost ${Boost_FIND_VERSION} COMPONENTS date_time thread)
MACRO_CHECK_SHOULD_BUILD(febird
  BZIP2_FOUND
  ZLIB_FOUND
  Boost_FOUND
  Boost_DATE_TIME_FOUND
  Boost_THREAD_FOUND
  )

IF(febird_SHOULD_BUILD)

  FILE(GLOB febird_SRC
    *.cpp
    io/*.cpp
    rpc/*.cpp
    )

  INCLUDE_DIRECTORIES(
    ${BZIP2_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    )

  MACRO_ADD_LIBRARY(febird SRCS ${febird_SRC})
  TARGET_LINK_LIBRARIES(
    febird
    ${BZIP2_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${Boost_LIBRARIES}
    )

ENDIF(febird_SHOULD_BUILD)
