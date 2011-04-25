INCLUDE_DIRECTORIES(${TokyoCabinet_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Glog_INCLUDE_DIRS})
FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS
  system
  thread
  filesystem
  serialization
  unit_test_framework
  program_options
  )
  
SET(t_bean_SRC
  t_beansdb.cpp
  )

  ADD_EXECUTABLE(t_beansdb ${t_bean_SRC})
  TARGET_LINK_LIBRARIES(t_beansdb
   beansdb
   izene_util
   ${Boost_LIBRARIES}
   ${Glog_LIBRARIES}
   procmeminfo
    )

