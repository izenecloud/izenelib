ADD_EXECUTABLE(t_fp_index
  Runner.cpp
  dup_det/t_dup_det.cpp
  )

TARGET_LINK_LIBRARIES(t_fp_index
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
  )

ADD_SUBDIRECTORY(index_manager)

