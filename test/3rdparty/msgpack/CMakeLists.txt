FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS
  system
  thread
  filesystem
  serialization
  unit_test_framework
  program_options
  )
  
FIND_PACKAGE(Threads)

INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${IZENELIB_SOURCE_DIR}/include/3rdparty/msgpack
    ${IZENELIB_SOURCE_DIR}/include/3rdparty/msgpack/rpc
    ${IZENELIB_SOURCE_DIR}/include/3rdparty/msgpack/rpc/transport
    ${IZENELIB_SOURCE_DIR}/include/3rdparty/msgpack/cclog
    #${IZENELIB_SOURCE_DIR}/include/3rdparty/msgpack/mp
    ${IZENELIB_SOURCE_DIR}/include/3rdparty/msgpack/mpsrc
    )
    
SET(libs
   msgpack
   febird
   ${CMAKE_THREAD_LIBS_INIT}
   izene_util
   ${Boost_LIBRARIES}
   ${Glog_LIBRARIES}
   procmeminfo
)

SET(t_msgpack_SRC
  t_msgpack.cc
  )

  ADD_EXECUTABLE(t_msgpack ${t_msgpack_SRC})
  TARGET_LINK_LIBRARIES(t_msgpack
   ${libs}
    )

ADD_EXECUTABLE(t_msgpack_server t_msgpack_server.cc)
TARGET_LINK_LIBRARIES(t_msgpack_server ${libs})

ADD_EXECUTABLE(t_msgpack_client t_msgpack_client.cc)
TARGET_LINK_LIBRARIES(t_msgpack_client ${libs})

