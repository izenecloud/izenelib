FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS
  system
  thread
  filesystem
  serialization
  unit_test_framework
  program_options
  )
LIST(FIND REAL_BUILD_COMPONENTS "boost_memory" _boostmemory)
LIST(FIND REAL_BUILD_COMPONENTS "index_manager" _indexmanager)

IF(Boost_FOUND)

INCLUDE_DIRECTORIES(
  ${Boost_INCLUDE_DIRS}
  ${TokyoCabinet_INCLUDE_DIRS}
  ${Glog_INCLUDE_DIRS}
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

SET(t_mock_indexer_SRC
  t_MockIndexReader.cpp
  t_master_suite.cpp
)

ADD_EXECUTABLE(t_mockindexer ${t_mock_indexer_SRC})

TARGET_LINK_LIBRARIES(t_mockindexer
  mock_index_manager
  index_manager
  boost_memory
  izene_util
  ${Glog_LIBRARIES}
  ${Boost_LIBRARIES}
)

SET(t_indexer_util_SRC
  t_priorityqueue.cpp
  t_bitvector.cpp
  t_master_suite.cpp
)

ADD_EXECUTABLE(t_indexer_util ${t_indexer_util_SRC})

TARGET_LINK_LIBRARIES(t_indexer_util
  index_manager
  boost_memory
  izene_util
  ${Glog_LIBRARIES}
  ${Boost_LIBRARIES}
)

SET(t_indexsort_SRC
  t_indexsort.cpp
  t_master_suite.cpp
)

ADD_EXECUTABLE(t_indexsort ${t_indexsort_SRC})

TARGET_LINK_LIBRARIES(t_indexsort
  index_manager
  boost_memory
  izene_util
  ${Glog_LIBRARIES}
  ${Boost_LIBRARIES}
)

SET(t_IndexMergePolicy_SRC
  t_IndexMergePolicy.cpp
  IndexerTestFixture.cpp
  t_master_suite.cpp
)

ADD_EXECUTABLE(t_IndexMergePolicy ${t_IndexMergePolicy_SRC})

TARGET_LINK_LIBRARIES(t_IndexMergePolicy
  index_manager
  boost_memory
  izene_util
  ${Glog_LIBRARIES}
  ${Boost_LIBRARIES}
)

SET(t_integration_Indexer_SRC
  t_integration_Indexer.cpp
  IndexerTestFixture.cpp
  t_TermDocFreqs.cpp
)

ADD_EXECUTABLE(t_integration_Indexer ${t_integration_Indexer_SRC})

TARGET_LINK_LIBRARIES(t_integration_Indexer
  index_manager
  boost_memory
  izene_util
  ${Glog_LIBRARIES}
  ${Boost_LIBRARIES}
)

ENDIF(Boost_FOUND)
