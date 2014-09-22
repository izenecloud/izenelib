FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS
  system
  thread
  filesystem
  serialization
  unit_test_framework
  program_options
  )
LIST(FIND REAL_BUILD_COMPONENTS "index_manager" _indexmanager)

SET(libs
  febird
  luxio
  index_manager
  izene_util
  procmeminfo
  am
  leveldb
  compressor
  ${Glog_LIBRARIES}
  ${Boost_LIBRARIES}
  ${TokyoCabinet_LIBRARIES}
  )

IF(Boost_FOUND)

INCLUDE_DIRECTORIES(
  ${Boost_INCLUDE_DIRS}
  ${TokyoCabinet_INCLUDE_DIRS}
  ${Glog_INCLUDE_DIRS}
  )

SET(t_inMemoryBarrel_SRC
  t_InMemoryBarrel.cpp
  t_master_suite.cpp
  )

ADD_EXECUTABLE(t_inMemoryBarrel ${t_inMemoryBarrel_SRC})

TARGET_LINK_LIBRARIES(t_inMemoryBarrel ${libs})

SET(t_indexer_SRC
  t_Indexer.cpp
  t_master_suite.cpp
  )

ADD_EXECUTABLE(t_indexer ${t_indexer_SRC})

TARGET_LINK_LIBRARIES(t_indexer ${libs})

SET(t_mock_indexer_SRC
  t_MockIndexReader.cpp
  t_master_suite.cpp
  )

ADD_EXECUTABLE(t_mockindexer ${t_mock_indexer_SRC})

TARGET_LINK_LIBRARIES(t_mockindexer ${libs} mock_index_manager)

SET(t_indexer_util_SRC
  t_priorityqueue.cpp
  t_bitvector.cpp
  t_master_suite.cpp
  )

ADD_EXECUTABLE(t_indexer_util ${t_indexer_util_SRC})

TARGET_LINK_LIBRARIES(t_indexer_util ${libs})

SET(t_EWAHTermDocFreqs_SRC
  t_EWAHTermDocFreqs.cpp
  t_master_suite.cpp
  )

ADD_EXECUTABLE(t_EWAHTermDocFreqs ${t_EWAHTermDocFreqs_SRC})

TARGET_LINK_LIBRARIES(t_EWAHTermDocFreqs ${libs})

SET(t_BitsetTermDocFreqs_SRC
  t_BitsetTermDocFreqs.cpp
  t_master_suite.cpp
  )

ADD_EXECUTABLE(t_BitsetTermDocFreqs ${t_BitsetTermDocFreqs_SRC})

TARGET_LINK_LIBRARIES(t_BitsetTermDocFreqs ${libs})

SET(t_indexsort_SRC
  t_indexsort.cpp
  t_master_suite.cpp
  )

ADD_EXECUTABLE(t_indexsort ${t_indexsort_SRC})

TARGET_LINK_LIBRARIES(t_indexsort ${libs})

SET(t_IndexMergePolicy_SRC
  t_IndexMergePolicy.cpp
  IndexerTestFixture.cpp
  t_master_suite.cpp
  )

ADD_EXECUTABLE(t_IndexMergePolicy ${t_IndexMergePolicy_SRC})

TARGET_LINK_LIBRARIES(t_IndexMergePolicy ${libs})

SET(t_integration_Indexer_SRC
  t_integration_Indexer.cpp
  IndexerTestFixture.cpp
  t_TermDocFreqs.cpp
  )

ADD_EXECUTABLE(t_integration_Indexer ${t_integration_Indexer_SRC})

TARGET_LINK_LIBRARIES(t_integration_Indexer ${libs})

SET(t_FSIndexInput_SRC
  t_FSIndexInput.cpp
  t_master_suite.cpp
  )

ADD_EXECUTABLE(t_FSIndexInput ${t_FSIndexInput_SRC})

TARGET_LINK_LIBRARIES(t_FSIndexInput ${libs})

ADD_EXECUTABLE(r_IndexStats r_IndexStats.cpp)

TARGET_LINK_LIBRARIES(r_IndexStats ${libs})

SET(t_btreeindexer_SRC
  rtype/t_BTreeIndexer.cpp
  t_master_suite.cpp
  )
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${IZENELIB_SOURCE_DIR}/testbin/rtypeindex)
ADD_EXECUTABLE(t_BTreeIndexer ${t_btreeindexer_SRC})
TARGET_LINK_LIBRARIES(t_BTreeIndexer ${libs})

SET(t_BitSet_SRC
  rtype/t_BitSet.cpp
  t_master_suite.cpp
  )
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${IZENELIB_SOURCE_DIR}/testbin/rtypeindex)
ADD_EXECUTABLE(t_BitSet ${t_BitSet_SRC})
TARGET_LINK_LIBRARIES(t_BitSet ${libs})

SET(btreeindexercmd_SRC
  rtype/BTreeIndexerCmd.cpp
  )
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${IZENELIB_SOURCE_DIR}/testbin/rtypeindex)
ADD_EXECUTABLE(BTreeIndexerCmd ${btreeindexercmd_SRC})
TARGET_LINK_LIBRARIES(BTreeIndexerCmd ${libs})

ENDIF(Boost_FOUND)
