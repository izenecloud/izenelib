/* 
 * File:   ConnectionPoolTest.cpp
 * Author: Paolo D'Apice
 *
 * Created on January 31, 2011, 15:28 PM
 */

#define BOOST_TEST_MODULE ConnectionPoolTest
#include <boost/test/unit_test.hpp>

#include "common.h"
#include "net/sf1r/ConnectionPool.hpp"
#include "net/sf1r/Errors.hpp"
#include "net/sf1r/RawClient.hpp"
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/thread.hpp>

using namespace NS_IZENELIB_SF1R;
using namespace std;


namespace {
ba::io_service service;
string host = "localhost";
string port = "18181";
size_t timeout = 60;
}

void test_delete_pool(ConnectionPool* pool)
{
    std::cout << "deleting in thread : " << pthread_self() << std::endl;
    if (pool)
        delete pool;
}

BOOST_AUTO_TEST_CASE(connection_fail) {
    BOOST_CHECK_THROW(ConnectionPool(service, "somewhere", "8888", timeout, 3).acquire(), NetworkError);
    BOOST_CHECK_THROW(ConnectionPool(service, "localhost", "12345", timeout, 3).acquire(), NetworkError);
    try
    {
    std::cout << "creating in thread : " << pthread_self() << std::endl;
    ConnectionPool *newpool = new ConnectionPool(service, "localhost", "18181", timeout, 3);
    boost::thread t(boost::bind(&test_delete_pool, newpool));
    t.join();
    }
    catch(const NetworkError& e)
    {
    }
}


/*
 * This test requires a running SF1.
 */

#ifdef ENABLE_SF1_TEST


/// Build a new request
inline string
getMessage(const uint32_t& seq) {
    return "{\"header\":{\"controller\":\"test\",\"action\":\"echo\"},"
                          "\"message\":" + boost::lexical_cast<string>(seq) + "}"; 
}


/** Test pool acquire/release. */
BOOST_AUTO_TEST_CASE(sanity) {
    const size_t SIZE = 2;
    
    ConnectionPool pool(service, host, port, timeout, SIZE, false);
    BOOST_CHECK(pool.invariant());
    BOOST_CHECK_EQUAL(SIZE, pool.getSize());
    BOOST_CHECK_EQUAL(SIZE, pool.getAvailableSize());
    BOOST_CHECK_EQUAL(0, pool.getReservedSize());
    BOOST_CHECK(not pool.isResizable());
    BOOST_CHECK(not pool.isBusy());
    
    // acquire
    
    RawClient& c1 = pool.acquire();
    BOOST_CHECK(pool.invariant());
    BOOST_CHECK_EQUAL(SIZE, pool.getSize());
    BOOST_CHECK_EQUAL(SIZE - 1, pool.getAvailableSize());
    BOOST_CHECK_EQUAL(1, pool.getReservedSize());
    BOOST_CHECK(c1.isConnected() and c1.idle());
    BOOST_CHECK(pool.isBusy());
    
    RawClient& c2 = pool.acquire();
    BOOST_CHECK(pool.invariant());
    BOOST_CHECK_EQUAL(SIZE, pool.getSize());
    BOOST_CHECK_EQUAL(SIZE - 2, pool.getAvailableSize());
    BOOST_CHECK_EQUAL(2, pool.getReservedSize());
    BOOST_CHECK(c2.isConnected() and c2.idle());
    BOOST_CHECK(pool.isBusy());
    
    // use
    
    c1.sendRequest(1u, getMessage(1u));
    BOOST_CHECK_EQUAL(RawClient::Busy, c1.getStatus());
    BOOST_CHECK(pool.invariant());
    
    c2.sendRequest(2u, getMessage(2u));
    BOOST_CHECK_EQUAL(RawClient::Busy, c2.getStatus());
    BOOST_CHECK(pool.invariant());
    
    c2.getResponse();
    BOOST_CHECK_EQUAL(RawClient::Idle, c2.getStatus());
    BOOST_CHECK(pool.invariant());
    
    // simulate network error
    
    c1.close();
    BOOST_CHECK_THROW(c1.getResponse(), runtime_error);
    BOOST_CHECK_EQUAL(RawClient::Invalid, c1.getStatus());
    BOOST_CHECK(pool.invariant());
    
    try {
        pool.acquire();
        BOOST_FAIL("ConnectionPoolError expected");
    } catch (ConnectionPoolError& e) {
        BOOST_CHECK_EQUAL("No available client (no resize)", e.what());
        BOOST_CHECK(pool.invariant());
    }
    
    // release
    
    pool.release(c1);
    BOOST_CHECK(pool.invariant());
    BOOST_CHECK_EQUAL(SIZE - 1, pool.getSize());
    BOOST_CHECK_EQUAL(SIZE - 2, pool.getAvailableSize());
    BOOST_CHECK_EQUAL(1, pool.getReservedSize());
    BOOST_CHECK(pool.isBusy());
    
    // check invalid replacement
    
    RawClient& c3 = pool.acquire();
    BOOST_CHECK(pool.invariant());
    BOOST_CHECK_EQUAL(SIZE, pool.getSize());
    BOOST_CHECK_EQUAL(SIZE - 2, pool.getAvailableSize());
    BOOST_CHECK_EQUAL(2, pool.getReservedSize());
    BOOST_CHECK(c3.isConnected() and c3.idle());
    BOOST_CHECK(pool.isBusy());
    
    pool.release(c3);
    BOOST_CHECK(pool.invariant());
    BOOST_CHECK_EQUAL(SIZE, pool.getSize());
    BOOST_CHECK_EQUAL(SIZE - 1, pool.getAvailableSize());
    BOOST_CHECK_EQUAL(1, pool.getReservedSize());
    BOOST_CHECK(pool.isBusy());
    
    pool.release(c2);
    BOOST_CHECK(pool.invariant());
    BOOST_CHECK_EQUAL(SIZE, pool.getSize());
    BOOST_CHECK_EQUAL(SIZE, pool.getAvailableSize());
    BOOST_CHECK_EQUAL(0, pool.getReservedSize());
    BOOST_CHECK(not pool.isBusy());
}


/** Test pool automatic resize. */
BOOST_AUTO_TEST_CASE(resize) {
    size_t SIZE = 1;
    const size_t MAX_SIZE = 3;
    
    ConnectionPool pool(service, host, port, timeout, SIZE, true, MAX_SIZE);
    BOOST_CHECK(pool.invariant());
    BOOST_CHECK_EQUAL(SIZE, pool.getSize());
    BOOST_CHECK_EQUAL(SIZE, pool.getAvailableSize());
    BOOST_CHECK_EQUAL(0, pool.getReservedSize());
    BOOST_CHECK(pool.isResizable());
    
    // acquire

    RawClient& c1 = pool.acquire();
    BOOST_CHECK(pool.invariant());
    BOOST_CHECK_EQUAL(SIZE, pool.getSize());
    BOOST_CHECK_EQUAL(SIZE - 1, pool.getAvailableSize());
    BOOST_CHECK_EQUAL(1, pool.getReservedSize());
    BOOST_CHECK(c1.isConnected() and c1.idle());
    
    RawClient& c2 = pool.acquire();
    BOOST_CHECK(pool.invariant());
    SIZE++; // auto incremented
    BOOST_CHECK_EQUAL(SIZE, pool.getSize());
    BOOST_CHECK_EQUAL(SIZE - 2, pool.getAvailableSize());
    BOOST_CHECK_EQUAL(2, pool.getReservedSize());
    BOOST_CHECK(c2.isConnected() and c2.idle());

    RawClient& c3 = pool.acquire();
    BOOST_CHECK(pool.invariant());
    SIZE++; // auto increment
    BOOST_CHECK_EQUAL(SIZE, pool.getSize());
    BOOST_CHECK_EQUAL(SIZE - 3, pool.getAvailableSize());
    BOOST_CHECK_EQUAL(3, pool.getReservedSize());
    BOOST_CHECK(c3.isConnected() and c3.idle());
    
    // use
    c1.sendRequest(1u, getMessage(1u));
    BOOST_CHECK_EQUAL(RawClient::Busy, c1.getStatus());
    
    c2.sendRequest(2u, getMessage(2u));
    BOOST_CHECK_EQUAL(RawClient::Busy, c2.getStatus());
    c2.getResponse();
    BOOST_CHECK_EQUAL(RawClient::Idle, c2.getStatus());
    
    c1.getResponse();
    BOOST_CHECK_EQUAL(RawClient::Idle, c1.getStatus());
    
    c3.sendRequest(3u, getMessage(3u));
    BOOST_CHECK_EQUAL(RawClient::Busy, c3.getStatus());
    
    try {
        pool.acquire();
        BOOST_FAIL("ConnectionPoolError expected");
    } catch (ConnectionPoolError& e) {
        BOOST_CHECK_EQUAL("No available client (max size reached)", e.what());
        // pool status unchanged
        BOOST_CHECK(pool.invariant());
        BOOST_CHECK_EQUAL(SIZE, pool.getSize());
        BOOST_CHECK_EQUAL(SIZE - 3, pool.getAvailableSize());
        BOOST_CHECK_EQUAL(3, pool.getReservedSize());
    }
    
    c3.getResponse();
    BOOST_CHECK_EQUAL(RawClient::Idle, c3.getStatus());
    
    // release
    
    pool.release(c3);
    BOOST_CHECK(pool.invariant());
    BOOST_CHECK_EQUAL(SIZE, pool.getSize());
    BOOST_CHECK_EQUAL(SIZE - 2, pool.getAvailableSize());
    BOOST_CHECK_EQUAL(2, pool.getReservedSize());
    
    pool.release(c2);
    BOOST_CHECK(pool.invariant());
    BOOST_CHECK_EQUAL(SIZE, pool.getSize());
    BOOST_CHECK_EQUAL(SIZE - 1, pool.getAvailableSize());
    BOOST_CHECK_EQUAL(1, pool.getReservedSize());
    
    pool.release(c1);
    BOOST_CHECK(pool.invariant());
    BOOST_CHECK_EQUAL(SIZE, pool.getSize());
    BOOST_CHECK_EQUAL(SIZE, pool.getAvailableSize());
    BOOST_CHECK_EQUAL(0, pool.getReservedSize());
}


struct Worker {
    Worker(ConnectionPool& p) : pool(p) {}
    void work(int i) {
        RawClient& c = pool.acquire();
        BOOST_CHECK(pool.invariant());
        
        c.sendRequest(i, getMessage(i));
        int sleepTime = (std::rand() % 5) + 1;
        LOG(INFO) << "worker " << i << " sleeping for " << sleepTime << " secs";
        boost::this_thread::sleep(boost::posix_time::seconds(sleepTime));
        c.getResponse();
        
        pool.release(c);
        BOOST_CHECK(pool.invariant());
        
        LOG(INFO) << "worker " << i << " done";
    }
private:    
    ConnectionPool& pool;
};


/** Simulate a concurrency test using threads. */
BOOST_AUTO_TEST_CASE(concurrency) {
    using boost::thread;

    const size_t NUM_THREADS = 5;
    size_t SIZE = 1;
    
    ConnectionPool pool(service, host, port, timeout, SIZE, true, NUM_THREADS);
    BOOST_CHECK(pool.invariant());
    
    Worker w(pool);
    
    boost::ptr_vector<thread> threads;
    for (size_t i = 0; i < NUM_THREADS; ++i) {
        threads.push_back(new thread(&Worker::work, &w, i+1));
    }
    
    BOOST_MESSAGE("Waiting for threads completion before termination");
}

#endif
