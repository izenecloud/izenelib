macro_check_should_build(
    sf1r
    Boost_FOUND
    Glog_FOUND
)

if (sf1r_SHOULD_BUILD)

    set(libsf1r_SOURCES
        RawClient.cpp
        ConnectionPool.cpp
        Sf1Driver.cpp
        JsonWriter.cpp
    )

    include_directories(
        ${Boost_INCLUDE_DIRS}
        ${Glog_INCLUDE_DIRS}
    )

    add_library(sf1r SHARED ${libsf1r_SOURCES})
    target_link_libraries(sf1r ${LIBS})
    install(TARGETS sf1r
            LIBRARY DESTINATION lib
            COMPONENT sf1r)
endif(sf1r_SHOULD_BUILD)
