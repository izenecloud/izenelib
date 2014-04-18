FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS
  system
  thread
  filesystem
  serialization
  unit_test_framework
  regex
  )
LIST(FIND REAL_BUILD_COMPONENTS "boost_memory" _boostmemory)

IF(Boost_FOUND)

  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${TokyoCabinet_INCLUDE_DIRS}
    ${Glog_INCLUDE_DIRS}
    )

  SET(t_idm_SRC
    t_IDManager.cpp
    t_WildcardQueryManager.cpp
    t_master_suite.cpp
    )

  ADD_EXECUTABLE(t_idm ${t_idm_SRC})

  TARGET_LINK_LIBRARIES(t_idm
    am
    izene_util
    febird
    jemalloc
    ${Glog_LIBRARIES}
    ${TokyoCabinet_LIBRARIES}
    ${Boost_LIBRARIES}
    leveldb
    compressor
    )

ENDIF(Boost_FOUND)
