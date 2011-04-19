INCLUDE_DIRECTORIES(${TokyoCabinet_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Glog_INCLUDE_DIRS})

SET(t_bean_SRC
  t_beansdb.cpp
  )

  ADD_EXECUTABLE(t_beansdb ${t_bean_SRC})
  TARGET_LINK_LIBRARIES(t_beansdb
   beansdb
   ${Boost_LIBRARIES}
   procmeminfo
    )

