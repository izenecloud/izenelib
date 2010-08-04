FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS
  system
  thread
  filesystem
  serialization
  unit_test_framework
  )
LIST(FIND REAL_BUILD_COMPONENTS "boost_memory" _boostmemory)
LIST(FIND REAL_BUILD_COMPONENTS "index_manager" _indexmanager)

IF(Boost_FOUND)

INCLUDE_DIRECTORIES(
  ${Boost_INCLUDE_DIRS}
 )

SET(t_indexer_SRC
  t_Indexer.cpp
  t_master_suite.cpp
)

ADD_EXECUTABLE(t_indexer ${t_indexer_SRC})

TARGET_LINK_LIBRARIES(t_indexer
  index_manager
  boost_memory
  izene_util
  ${Glog_LIBRARIES}
  ${Boost_LIBRARIES}
)

ENDIF(Boost_FOUND)
