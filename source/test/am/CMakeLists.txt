LIST(FIND REAL_BUILD_COMPONENTS "febird" _febird)

IF (TokyoCabinet_FOUND AND Glog_FOUND AND _febird GREATER -1)

  INCLUDE_DIRECTORIES(${TokyoCabinet_INCLUDE_DIRS})
  INCLUDE_DIRECTORIES(${Glog_INCLUDE_DIRS})

  SET(t_am_SRC
    Runner.cpp

    IntString.cpp
    IntNull.cpp
    OpenClose.cpp

    raw/Buffer.cpp

    tc/String.cpp
    tc/Iter.cpp
    tc/raw/Db.cpp
    tc/raw/Iter.cpp
    )

  ADD_EXECUTABLE(t_am ${t_am_SRC})
  TARGET_LINK_LIBRARIES(t_am
    febird
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${Boost_SERIALIZATION_LIBRARY}
    ${TokyoCabinet_LIBRARIES}
    ${Glog_LIBRARIES}
    )

  ADD_EXECUTABLE(t_fmm vsynonym/t_fmm.cpp )
  TARGET_LINK_LIBRARIES(t_fmm
    am
    febird
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${Boost_SERIALIZATION_LIBRARY}
    ${TokyoCabinet_LIBRARIES}
    ${Glog_LIBRARIES}
    )

ENDIF (TokyoCabinet_FOUND AND Glog_FOUND AND _febird GREATER -1)
