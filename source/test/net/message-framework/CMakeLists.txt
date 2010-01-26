SET(_NOT_WIN32 TRUE)
IF(WIN32)
  SET(_NOT_WIN32 FALSE)
ENDIF(WIN32)

FIND_PACKAGE(Boost REQUIRED COMPONENTS
  system
  thread
  serialization
  unit_test_framework
)

FIND_PACKAGE(izenelib REQUIRED COMPONENTS
  message_framework
  procmeminfo
)

MACRO_CHECK_SHOULD_BUILD(
  message_framework
  _NOT_WIN32
  Boost_FOUND
  wiselib_FOUND
  Glog_FOUND
)


INCLUDE_DIRECTORIES(
	${Boost_INCLUDE_DIRS}
	${izenelib_INCLUDE_DIRS}
	${wiselib_INCLUDE_DIRS}
	${Glog_INCLUDE_DIRS}
)

SET(t_mf_SRC
	t_ServiceResolver.cpp
	t_master_suite.cpp
)

ADD_EXECUTABLE(t_mf ${t_mf_SRC})

TARGET_LINK_LIBRARIES(t_mf
	${wiselib_LIBRARIES}
	${izenelib_LIBRARIES}
	${Boost_LIBRARIES}
	${Glog_LIBRARIES}
)

SET_TARGET_PROPERTIES(t_mf PROPERTIES
	COMPILE_FLAGS "-include ${FileTestHelper}"
	RUNTIME_OUTPUT_DIRECTORY ${IZENELIB_SOURCE_DIR}/testbin
)


ADD_TEST(mf "${IZENELIB_SOURCE_DIR}/testbin/t_mf")
