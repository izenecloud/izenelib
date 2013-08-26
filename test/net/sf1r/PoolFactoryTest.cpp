/* 
 * File:   PoolFactoryTest.cpp
 * Author: Paolo D'Apice
 *
 * Created on March 31, 2011, 10:02 AM
 */

#define BOOST_TEST_MODULE PoolFactoryTest
#include <boost/test/unit_test.hpp>

#include "common.h"
#include "net/sf1r/Errors.hpp"
#include "net/sf1r/Sf1Config.hpp"
#include "net/sf1r/PoolFactory.hpp"

using namespace NS_IZENELIB_SF1R;
using namespace std;


namespace {
ba::io_service service;
Sf1Config conf;
}


BOOST_AUTO_TEST_CASE(constructor) {
    BOOST_CHECK_NO_THROW(PoolFactory(service, conf));
    BOOST_CHECK_NO_THROW(PoolFactory(service, conf));
}


BOOST_AUTO_TEST_CASE(connection_fail) {
    PoolFactory factory(service, conf);
    BOOST_CHECK_THROW(factory.newConnectionPool("somewhere")->acquire(), NetworkError);
    
    Sf1Node node("/test/path","collection#foo$dataport#18121$baport#18181$masterport#18131$host#somewhere");
    BOOST_CHECK_THROW(factory.newConnectionPool(node)->acquire(), NetworkError);
}


/*
 * This test requires a running SF1.
 */

#ifdef ENABLE_SF1_TEST

BOOST_AUTO_TEST_CASE(connection) {
    PoolFactory factory(service, conf);
    ConnectionPool* pool;
    BOOST_CHECK_NO_THROW(pool = factory.newConnectionPool("localhost:18181"));
    delete pool;
    
    Sf1Node node("/test/path","collection#foo$dataport#18121$baport#18181$masterport#18131$host#localhost");
    BOOST_CHECK_NO_THROW(pool = factory.newConnectionPool(node));
    delete pool;
}

#endif
