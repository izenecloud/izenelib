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
  ${Glog_LIBRARIES}
  ${Boost_LIBRARIES}
  izene_util
  febird
  am
  zambezi
)

IF(Boost_FOUND)

INCLUDE_DIRECTORIES(
  ${Boost_INCLUDE_DIRS}
  ${Glog_INCLUDE_DIRS}
 )

SET(t_index_SRC
  t_index_search.cpp
  t_master_suite.cpp
)

ADD_EXECUTABLE(t_ZambeziSearch ${t_index_SRC})

TARGET_LINK_LIBRARIES(t_ZambeziSearch ${libs})

ENDIF(Boost_FOUND)
