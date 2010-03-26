FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS thread)

MACRO_CHECK_SHOULD_BUILD(
  luxio
  Boost_FOUND
  )

IF(luxio_SHOULD_BUILD)

  FILE(GLOB_RECURSE
    luxio_SRC
    RELATIVE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "*.cpp"
    )

  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include/3rdparty/am/luxio
    )

  ADD_DEFINITIONS("-fPIC")

  ADD_LIBRARY(luxio ${luxio_SRC})
  INSTALL(TARGETS luxio
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib
      COMPONENT "izenelib_libraries")

ENDIF(luxio_SHOULD_BUILD)

