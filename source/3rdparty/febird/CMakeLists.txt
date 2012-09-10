FIND_PACKAGE(Boost ${Boost_FIND_VERSION} COMPONENTS date_time thread)
MACRO_CHECK_SHOULD_BUILD(febird
  BZIP2_FOUND
  ZLIB_FOUND
  Boost_FOUND
  Boost_DATE_TIME_FOUND
  Boost_THREAD_FOUND
  )

IF(febird_SHOULD_BUILD)

  SET(febird_SRC
    io/access_byid.cpp
    io/BzipStream.cpp
    io/DataInput.cpp
    io/DataIO_Exception.cpp
    io/FileStream.cpp
    io/HexCodingStream.cpp
    io/IOException.cpp
    io/IStream.cpp
    io/mem_map_stream.cpp
    io/MemStream.cpp
    io/OutputStream.cpp
    io/SocketStream.cpp
    io/StreamBuffer.cpp
    io/var_int.cpp
    io/WinFileStream.cpp
    io/ZlibStream.cpp
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
