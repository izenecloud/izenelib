#cannot compile
#ADD_EXECUTABLE(t_exception t_exception)

ADD_EXECUTABLE(t_map t_map)

IF(febird_shared AND Boost_FOUND)
  ADD_EXECUTABLE(t_serialization t_serialization)
  TARGET_LINK_LIBRARIES(t_serialization febird_shared)
ENDIF(febird_shared AND Boost_FOUND)

FIND_PACKAGE(Boost 1.35.0 COMPONENTS unit_test_framework)
IF(Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)
  ADD_EXECUTABLE(t_ClockTimer t_ClockTimer)
  TARGET_LINK_LIBRARIES(t_ClockTimer ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
  ADD_TEST(util/ClockTimer t_ClockTimer)
ENDIF(Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)

#FIND_PACKAGE(Boost 1.35.0 COMPONENTS serialization)
#IF(Boost_FOUND AND Boost_SERIALIZATION_FOUND)
#  ADD_EXECUTABLE(t_wrapper t_wrapper)
#  TARGET_LINK_LIBRARIES(t_wrapper febird_shared ${Boost_SERIALIZATION_LIBRARY}  ${Boost_THREAD_LIBRARY})
#ENDIF(Boost_FOUND AND Boost_SERIALIZATION_FOUND)


