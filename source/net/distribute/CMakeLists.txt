MACRO_CHECK_SHOULD_BUILD(
  distribute
  Boost_FOUND
  Glog_FOUND
)

IF(distribute_SHOULD_BUILD)

  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${Glog_INCLUDE_DIRS}
  )

  SET(distribute_SRC
    DataReceiver.cpp
    DataTransfer.cpp
    Msg.cpp
    SocketIO.cpp
    DataTransfer2.cpp
    DataReceiver2.cpp
    Connection.cpp
  )

  ADD_DEFINITIONS("-fPIC")
  MACRO_ADD_LIBRARY(distribute SRCS ${distribute_SRC})
  
# TODO: remove izene_util
  TARGET_LINK_LIBRARIES(distribute
    izene_util
  )

ENDIF(distribute_SHOULD_BUILD)
