MACRO_CHECK_SHOULD_BUILD(
  boost_network
  Boost_FOUND
  )

IF(boost_network_SHOULD_BUILD)

  INCLUDE_DIRECTORIES(
    ${Boost_INCLUDE_DIRS}
    )

  SET(boost_network_SRC

    client.cpp
    server_request_parsers_impl.cpp
    uri/schemes.cpp
    uri/uri.cpp
    )


  MACRO_ADD_LIBRARY(boost_network SRCS ${boost_network_SRC})

ENDIF(boost_network_SHOULD_BUILD)
