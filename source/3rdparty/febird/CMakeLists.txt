IF(BZIP2_FOUND)

  SET_PROPERTY(GLOBAL APPEND PROPERTY BUILD_COMPONENTS febird)

  FILE(GLOB_RECURSE febird_SRC
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

  INCLUDE_DIRECTORIES(${BZIP2_INCLUDE_DIR})

  ADD_LIBRARY(febird_shared SHARED ${febird_SRC})
  SET_TARGET_PROPERTIES(febird_shared PROPERTIES
    OUTPUT_NAME "febird"
    CLEAN_DIRECT_OUTPUT 1
    VERSION ${IZENELIB_VERSION}
    )
  TARGET_LINK_LIBRARIES(febird_shared ${BZIP2_LIBRARIES})

  ADD_LIBRARY(febird_static STATIC ${febird_SRC})
  SET_TARGET_PROPERTIES(febird_static PROPERTIES
    OUTPUT_NAME "febird"
    CLEAN_DIRECT_OUTPUT 1
    )
  TARGET_LINK_LIBRARIES(febird_static ${BZIP2_LIBRARIES})

  INSTALL(TARGETS febird_static febird_shared
    EXPORT febird
    ARCHIVE DESTINATION ${VERSION_INSTALL_PREFIX}/lib
    LIBRARY DESTINATION shared)

  INSTALL(EXPORT febird
    DESTINATION ${VERSION_INSTALL_PREFIX}/cmake)

ELSE(BZIP2_FOUND)

  SET_PROPERTY(GLOBAL APPEND PROPERTY NON_BUILD_COMPONENTS febird)
  MESSAGE(STATUS "! WARNING: Febird cannot been built without bzip2")

ENDIF(BZIP2_FOUND)
