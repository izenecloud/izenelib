FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS
  system
  thread
  filesystem
  serialization
  random
  unit_test_framework
  )

SET(libs
  zambezi
  izene_util
  febird
  am
  ${Glog_LIBRARIES}
  ${Boost_LIBRARIES}
)

IF(Boost_FOUND)

INCLUDE_DIRECTORIES(
  ${Boost_INCLUDE_DIRS}
  ${Glog_INCLUDE_DIRS}
 )

SET(t_position_index_SRC
  t_position_inverted_index.cpp
  t_master_suite.cpp
)

SET(t_attr_score_index_SRC
  t_attr_score_inverted_index.cpp
  t_master_suite.cpp
)

ADD_EXECUTABLE(t_PositionZambeziSearch ${t_position_index_SRC})

TARGET_LINK_LIBRARIES(t_PositionZambeziSearch ${libs})


ADD_EXECUTABLE(t_AttrScoreZambeziSearch ${t_attr_score_index_SRC})

TARGET_LINK_LIBRARIES(t_AttrScoreZambeziSearch ${libs})

ENDIF(Boost_FOUND)
