FIND_PACKAGE(Boost 1.35.0 REQUIRED COMPONENTS unit_test_framework)

IF (Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

  IF (TokyoCabinet_FOUND)
    INCLUDE_DIRECTORIES(${TokyoCabinet_INCLUDE_DIRS})
    SET(t_am_SRC
      Runner.cpp
      
      IntStringAmTest.cpp

      raw/Buffer.cpp

      tc/String.cpp
      tc/raw/Db.cpp
      )

    ADD_EXECUTABLE(t_am ${t_am_SRC})
    TARGET_LINK_LIBRARIES(t_am
      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
      ${TokyoCabinet_LIBRARIES}
      )

    SET_TARGET_PROPERTIES(t_am PROPERTIES
      COMPILE_FLAGS "-include ${FileTestHelper}"
      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/testbin
      )

    ADD_TEST(am ${CMAKE_SOURCE_DIR}/testbin/t_am)
  ENDIF (TokyoCabinet_FOUND)

ENDIF (Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND AND TokyoCabinet_FOUND)


