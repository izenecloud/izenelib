
  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${IZENELIB_SOURCE_DIR}/include/3rdparty/am/
    )

SET(t_babudb_SRC
  t_babudb.cpp
  t_master_suite.cpp
  )

  ADD_EXECUTABLE(t_babudb ${t_babudb_SRC})
  TARGET_LINK_LIBRARIES(t_babudb
   babudb
   ${Boost_LIBRARIES}
   procmeminfo
    )

