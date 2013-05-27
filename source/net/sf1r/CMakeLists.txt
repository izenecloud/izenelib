macro_check_should_build(
    sf1r
    Boost_FOUND
    Glog_FOUND
)

if (sf1r_SHOULD_BUILD)

    find_package(Boost ${Boost_FIND_VERSION}
        REQUIRED COMPONENTS
        regex
        thread
        serialization
        system
        date_time
    )

    set(libsf1r_SOURCES
        Sf1DriverBase.cpp
        Sf1Driver.cpp
        RawClient.cpp
        ConnectionPool.cpp
        JsonWriter.cpp
        
        distributed/Sf1DistributedDriver.cpp
        distributed/Sf1Node.cpp
        distributed/Sf1Topology.cpp
        distributed/Sf1Watcher.cpp
        distributed/ZooKeeperRouter.cpp
        distributed/RoundRobinPolicy.cpp

        ${IZENELIB_SOURCE_DIR}/source/util/kv2string.cpp
        ${IZENELIB_SOURCE_DIR}/source/util/scheduler.cpp
        ${IZENELIB_SOURCE_DIR}/source/util/timer.cpp
        ${IZENELIB_SOURCE_DIR}/source/util/singleton.cpp
    )

    include_directories(
        ${Boost_INCLUDE_DIRS}
        ${Glog_INCLUDE_DIRS}
    )

    add_library(sf1r SHARED ${libsf1r_SOURCES})
    target_link_libraries(sf1r zookeeper ${LIBS} ${Boost_LIBRARIES})
    install(TARGETS sf1r
            LIBRARY DESTINATION lib
            COMPONENT sf1r)

    list(APPEND LIBS pthread rt)
    add_executable(Sf1rSharedDataMaster distributed/share_data_master.cpp)
    target_link_libraries(Sf1rSharedDataMaster ${LIBS} ${Boost_LIBRARIES}
        ${Glog_LIBRARIES})
    install(TARGETS Sf1rSharedDataMaster
            RUNTIME DESTINATION bin
            COMPONENT Sf1rSharedDataMaster)

endif(sf1r_SHOULD_BUILD)
