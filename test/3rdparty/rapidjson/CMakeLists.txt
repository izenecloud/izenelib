FIND_PACKAGE(Boost ${Boost_FIND_VERSION}
  COMPONENTS
  system
  thread
  filesystem
  unit_test_framework
)

INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    ${IZENELIB_SOURCE_DIR}/include/3rdparty/
)

SET(libs
   izene_util
   febird
   ${Boost_LIBRARIES}
   ${Glog_LIBRARIES}
)

SET(t_rapidjson_SRC
  Runner.cpp
  t_rapidjson.cpp
)

ADD_EXECUTABLE(t_rapidjson ${t_rapidjson_SRC})
TARGET_LINK_LIBRARIES(t_rapidjson ${libs})

ADD_EXECUTABLE(rapidjson_example t_example.cpp)
TARGET_LINK_LIBRARIES(rapidjson_example ${libs})
