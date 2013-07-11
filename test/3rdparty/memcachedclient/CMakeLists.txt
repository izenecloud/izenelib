FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS
  system
  thread
  filesystem
  serialization
  unit_test_framework
  program_options
  )
  
INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${IZENELIB_SOURCE_DIR}/include/3rdparty/memcachedclient
    )
    
SET(libs
   memcachedclient
   izene_util
   febird
   ${Boost_LIBRARIES}
   ${Glog_LIBRARIES}
)

SET(t_memcachedclient_SRC
  t_memcachedclient.cpp
  )


ADD_EXECUTABLE(t_memcachedclient ${t_memcachedclient_SRC})
TARGET_LINK_LIBRARIES(t_memcachedclient ${libs})
