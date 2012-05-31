macro_check_should_build(
  distribute
  Boost_FOUND
  Glog_FOUND
)

if(distribute_SHOULD_BUILD)

  find_package(Boost ${Boost_FIND_VERSION}
    REQUIRED COMPONENTS
    filesystem
    thread
  )

  include_directories(
    ${Boost_INCLUDE_DIRS}
    ${Glog_INCLUDE_DIRS}
  )

  set(distribute_SRC
    DataTransfer2.cpp
    DataReceiver2.cpp
    Connection.cpp
  )

  add_definitions("-fPIC")

  macro_add_library(distribute SRCS ${distribute_SRC})
  target_link_libraries(distribute msgpack ${Boost_LIBRARIES})

endif(distribute_SHOULD_BUILD)
