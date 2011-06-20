MACRO_CHECK_SHOULD_BUILD(msgpack Threads_FOUND)

IF(msgpack_SHOULD_BUILD)

    FILE(GLOB msgpack_SRC
    *.cpp
    *.c
    rpc/*.cc
    rpc/*.c
    rpc/transport/*.cc
    cclog/*.cc
    mpsrc/wavy_connect.cc
    mpsrc/wavy_listen.cc
    mpsrc/wavy_loop.cc
    mpsrc/wavy_signal.cc
    mpsrc/wavy_timer.cc
    )

  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${IZENELIB_SOURCE_DIR}/include
    )

  ADD_DEFINITIONS("-fPIC")

  MACRO_ADD_LIBRARY(msgpack SRCS ${msgpack_SRC})

ENDIF(msgpack_SHOULD_BUILD)
