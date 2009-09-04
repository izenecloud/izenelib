FIND_PACKAGE(Boost 1.35.0 REQUIRED COMPONENTS unit_test_framework)

IF (Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)
  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/core/include)
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

  SET(t_SimilarityDetection_SRC
    Runner.cpp
    SimilarityListTest.cpp
    SimilarityIndexTest.cpp
    SimilarityDetectorTest.cpp
    )
  ADD_EXECUTABLE(t_SimilarityDetection ${t_SimilarityDetection_SRC})
  TARGET_LINK_LIBRARIES(t_SimilarityDetection
    sf1v5_document_manager
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    tokyocabinet
    )

  GET_FILENAME_COMPONENT(FileTestHelper "${CMAKE_CURRENT_SOURCE_DIR}/TestHelper.h" ABSOLUTE)
  SET_TARGET_PROPERTIES(t_SimilarityDetection PROPERTIES COMPILE_FLAGS "-include ${FileTestHelper}")

  ADD_TEST(SimilarityDetection t_SimilarityDetection)
ENDIF (Boost_FOUND AND Boost_UNIT_TEST_FRAMEWORK_FOUND)
