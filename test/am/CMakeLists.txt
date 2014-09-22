LIST(FIND REAL_BUILD_COMPONENTS "febird" _febird)

IF (TokyoCabinet_FOUND AND Glog_FOUND AND _febird GREATER -1)

  INCLUDE_DIRECTORIES(${TokyoCabinet_INCLUDE_DIRS})
  INCLUDE_DIRECTORIES(${Glog_INCLUDE_DIRS})
#  INCLUDE_DIRECTORIES(${Toku_INCLUDE_DIR})
  SET(t_am_SRC
    Runner.cpp

    #concurrent/t_skiplist.cpp
    concurrent/t_skiplist.cpp
    concurrent/t_slfvector.cpp

    #IntString.cpp
    #IntNull.cpp
    #OpenClose.cpp

    raw/Buffer.cpp

    tc/tc_hash.cpp
    tc/String.cpp
    tc/Iter.cpp
    tc/Db.cpp
    #tc/raw/Db.cpp
    #tc/raw/Iter.cpp

    luxio/Db.cpp
    leveldb/Db.cpp
    #tokukv/Db.cpp
    lmdb/Db.cpp

    graphchi/t_basic_smoketest.cpp
    graphchi/t_pagerank.cpp
    )
  ADD_DEFINITIONS("-fopenmp -Wunused-variable")
  ADD_EXECUTABLE(t_am ${t_am_SRC})
  TARGET_LINK_LIBRARIES(t_am
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    ${Boost_IOSTREAMS_LIBRARY}
    febird
    izene_util
    bz2
    ${Glog_LIBRARIES}
    luxio
    ${TokyoCabinet_LIBRARIES}
    #${Toku_LIBS}
    am
    leveldb
    compressor
    lmdb
    jemalloc
    gomp
    pthread
    )

  FILE(GLOB_RECURSE
    t_succinct_SRC
    RELATIVE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "succinct/*.cpp"
    )

  ADD_EXECUTABLE(t_succinct ${t_succinct_SRC})
  TARGET_LINK_LIBRARIES(t_succinct
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    ${Boost_IOSTREAMS_LIBRARY}
    am
    febird
    izene_util
    bz2
    jemalloc
    ${Glog_LIBRARIES}
    )

  ADD_EXECUTABLE(manual_t_fmm vsynonym/t_fmm.cpp )
  TARGET_LINK_LIBRARIES(manual_t_fmm
    ${CMAKE_THREAD_LIBS_INIT}
    am
    compressor
    febird
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${Boost_SERIALIZATION_LIBRARY}
    ${TokyoCabinet_LIBRARIES}
    ${Boost_SYSTEM_LIBRARY}
    ${Glog_LIBRARIES}
    )

  ADD_EXECUTABLE(t_app approximate_matching/t_app.cpp)
  TARGET_LINK_LIBRARIES(t_app
    ${Boost_LIBRARIES}
    am
    febird
    izene_util
    ${Glog_LIBRARIES}
    )

ENDIF (TokyoCabinet_FOUND AND Glog_FOUND AND Toku_FOUND AND _febird GREATER -1)

ADD_EXECUTABLE(t_izenesort
  Runner.cpp
  izene_sort/t_izene_sort.cpp
  )

TARGET_LINK_LIBRARIES(t_izenesort
  am
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Glog_LIBRARIES}
  febird
  )

ADD_EXECUTABLE(t_line_reader
  Runner.cpp
  util/t_line_reader.cpp
  )

TARGET_LINK_LIBRARIES(t_line_reader
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  )

ADD_EXECUTABLE(t_khashtable
  Runner.cpp
  hashtable/t_khashtable.cpp
  )

TARGET_LINK_LIBRARIES(t_khashtable
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  )

ADD_EXECUTABLE(t_hashtable
  Runner.cpp
  hashtable/t_hashtable.cpp
  )

TARGET_LINK_LIBRARIES(t_hashtable
  am
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Glog_LIBRARIES}
  )

ADD_EXECUTABLE(t_matrix
  Runner.cpp
  matrix/t_matrix_db.cpp
  )

TARGET_LINK_LIBRARIES(t_matrix
  am
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_IOSTREAMS_LIBRARY}
  ${TokyoCabinet_LIBRARIES}
  ${Boost_LIBRARIES}
  ${Glog_LIBRARIES}
  febird
  procmeminfo
  izene_log
  leveldb
  compressor
  izene_util
  )

ADD_EXECUTABLE(t_matrix_cache
  Runner.cpp
  matrix/t_matrix_cache.cpp
)

TARGET_LINK_LIBRARIES(t_matrix_cache
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
)

ADD_EXECUTABLE(t_bitmap
  Runner.cpp
  bitmap/t_ewah.cpp
  )

TARGET_LINK_LIBRARIES(t_bitmap
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${TokyoCabinet_LIBRARIES}
  febird
  izene_util
  index_manager
  luxio
  leveldb
  am
  compressor
  ${Glog_LIBRARIES}
  )
