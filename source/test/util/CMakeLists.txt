ADD_EXECUTABLE(t_ClockTimer t_ClockTimer)
ADD_TEST(util/ClockTimer t_ClockTimer)

#cannot compile
#ADD_EXECUTABLE(t_exception EXCLUDE_FROM_ALL t_exception)

ADD_EXECUTABLE(t_map t_map)

FIND_PACKAGE(Boost COMPONENTS serialization)

IF(Boost_FOUND AND Boost_SERIALIZATION_FOUND)
  ADD_EXECUTABLE(t_wrapper t_wrapper)
  TARGET_LINK_LIBRARIES(t_wrapper ${Boost_SERIALIZATION_LIBRARY})
ENDIF(Boost_FOUND AND Boost_SERIALIZATION_FOUND)

IF(febird_shared AND Boost_FOUND AND Boost_SERIALIZATION_FOUND)
  ADD_EXECUTABLE(t_serialization t_serialization)
  TARGET_LINK_LIBRARIES(t_serialization febird_shared)
ENDIF(febird_shared AND Boost_FOUND AND Boost_SERIALIZATION_FOUND)

