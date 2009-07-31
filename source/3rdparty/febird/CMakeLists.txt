IF(BZIP2_FOUND AND ZLIB_FOUND)

  SET_PROPERTY(GLOBAL APPEND PROPERTY BUILD_COMPONENTS febird)

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
    rpc/client.cpp
    rpc/MessageInputStream.cpp
    rpc/rpc_basic.cpp
    rpc/server.cpp
    rpc/server_io.cpp
    )

  INCLUDE_DIRECTORIES(${BZIP2_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})

  MACRO_ADD_COMPONENT(febird "${VERSION_INSTALL_PREFIX}" "${PROJECT_VERSION}" ${febird_SRC})
  TARGET_LINK_LIBRARIES(febird_shared ${BZIP2_LIBRARIES} ${ZLIB_LIBRARIES})
  TARGET_LINK_LIBRARIES(febird_static ${BZIP2_LIBRARIES} ${ZLIB_LIBRARIES})

ELSE(BZIP2_FOUND AND ZLIB_FOUND)

  SET_PROPERTY(GLOBAL APPEND PROPERTY NON_BUILD_COMPONENTS febird)
  MESSAGE(STATUS "! WARNING: Febird cannot been built without bzip2 and zlib")

ENDIF(BZIP2_FOUND AND ZLIB_FOUND)
