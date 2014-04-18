SET(t_seda_SRC
  Runner.cpp
  t_seda.cpp
  )

ADD_EXECUTABLE(tt_seda ${t_seda_SRC})
TARGET_LINK_LIBRARIES(tt_seda
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  pthread
  )
