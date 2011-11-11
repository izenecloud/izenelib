INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${IZENELIB_SOURCE_DIR}/include/3rdparty/am/
    )
FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS
  system
  thread
  filesystem
  serialization
  unit_test_framework
  program_options
  )
  
SET(t_dag_vector_SRC
  t_dag_vector.cpp
  t_sparse_set.cpp
  t_sparse_map.cpp
  t_comp_vector.cpp
  t_master_suite.cpp
  )

  ADD_EXECUTABLE(t_dag_vector ${t_dag_vector_SRC})
  TARGET_LINK_LIBRARIES(t_dag_vector
   izene_util
   febird
   ${Boost_LIBRARIES}
   ${Glog_LIBRARIES}
   procmeminfo
    )

