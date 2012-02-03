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
#include "net/sf1r/RawClient.hpp"
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/thread.hpp>

using namespace NS_IZENELIB_SF1R;
using ba::ip::tcp;
using namespace std;


#ifdef ENABLE_SF1_TEST // don't know if there is a running SF1

/** Test fixture. */
struct AsioService {
    AsioService() : host("localhost"), port("18181"), 
                    resolver(service), query(host, port) {
        iterator = resolver.resolve(query);
    }
    ~AsioService() {}
    
    const string host;
    const string port;
    
    ba::io_service service;
    tcp::resolver resolver;
    tcp::resolver::query query;
    tcp::resolver::iterator iterator;
};


inline string
getMessage(const uint32_t& seq) {
    return "{\"header\":{\"controller\":\"test\",\"action\":\"echo\"},"
                          "\"message\":" + boost::lexical_cast<string>(seq) + "}"; 
}


BOOST_FIXTURE_TEST_CASE(sanity_test, AsioService) {
    const size_t SIZE = 2;
    
    ConnectionPool pool(service, iterator, SIZE, false, SIZE);
    BOOST_CHECK(pool.invariant());
    BOOST_CHECK_EQUAL(SIZE, pool.getInfo().get<POOL_SIZE>());
    BOOST_CHECK_EQUAL(SIZE, pool.getInfo().get<POOL_AVAILABLE>());
    BOOST_CHECK_EQUAL(0, pool.getInfo().get<POOL_BUSY>());
    BOOST_CHECK_EQUAL(false, pool.getInfo().get<POOL_RESIZE_FLAG>());
    
    // acquire
    
    RawClient& c1 = pool.acquire();
    BOOST_CHECK(pool.invariant());
    BOOST_CHECK_EQUAL(SIZE, pool.getInfo().get<POOL_SIZE>());
    BOOST_CHECK_EQUAL(SIZE - 1, pool.getInfo().get<POOL_AVAILABLE>());
    BOOST_CHECK_EQUAL(1, pool.getInfo().get<POOL_BUSY>());
    BOOST_CHECK(c1.isConnected() and c1.idle());
    
    RawClient& c2 = pool.acquire();
    BOOST_CHECK(pool.invariant());
    BOOST_CHECK_EQUAL(SIZE, pool.getInfo().get<POOL_SIZE>());
    BOOST_CHECK_EQUAL(SIZE - 2, pool.getInfo().get<POOL_AVAILABLE>());
    BOOST_CHECK_EQUAL(2, pool.getInfo().get<POOL_BUSY>());
    BOOST_CHECK(c2.isConnected() and c2.idle());
    
    // use
    
    c1.sendRequest(1u, getMessage(1u));
    BOOST_CHECK_EQUAL(RawClient::Busy, c1.getStatus());
    
    c2.sendRequest(2u, getMessage(2u));
    BOOST_CHECK_EQUAL(RawClient::Busy, c2.getStatus());
    c2.getResponse();
    BOOST_CHECK_EQUAL(RawClient::Idle, c2.getStatus());
    
    c1.getResponse();
    BOOST_CHECK_EQUAL(RawClient::Idle, c1.getStatus());
    
    try {
        pool.acquire();
        BOOST_FAIL("ConnectionPoolError expected");
    } catch (ConnectionPoolError& e) {
        BOOST_CHECK_EQUAL("No available client (no resize)", e.what());
    }
    
    // release
    
    pool.release();
    BOOST_CHECK(pool.invariant());
    BOOST_CHECK_EQUAL(SIZE, pool.getInfo().get<POOL_SIZE>());
    BOOST_CHECK_EQUAL(SIZE - 1, pool.getInfo().get<POOL_AVAILABLE>());
    BOOST_CHECK_EQUAL(1, pool.getInfo().get<POOL_BUSY>());
    
    pool.release();
    BOOST_CHECK(pool.invariant());
    BOOST_CHECK_EQUAL(SIZE, pool.getInfo().get<POOL_SIZE>());
    BOOST_CHECK_EQUAL(SIZE, pool.getInfo().get<POOL_AVAILABLE>());
    BOOST_CHECK_EQUAL(0, pool.getInfo().get<POOL_BUSY>());
}


BOOST_FIXTURE_TEST_CASE(resize_test, AsioService) {
    size_t SIZE = 1;
    const size_t MAX_SIZE = 3;
    
    ConnectionPool pool(service, iterator, SIZE, true, MAX_SIZE);
    BOOST_CHECK(pool.invariant());
    BOOST_CHECK_EQUAL(SIZE, pool.getInfo().get<POOL_SIZE>());
    BOOST_CHECK_EQUAL(SIZE, pool.getInfo().get<POOL_AVAILABLE>());
    BOOST_CHECK_EQUAL(0, pool.getInfo().get<POOL_BUSY>());
    BOOST_CHECK_EQUAL(true, pool.getInfo().get<POOL_RESIZE_FLAG>());
    
    // acquire

    RawClient& c1 = pool.acquire();
    BOOST_CHECK(pool.invariant());
    BOOST_CHECK_EQUAL(SIZE, pool.getInfo().get<POOL_SIZE>());
    BOOST_CHECK_EQUAL(SIZE - 1, pool.getInfo().get<POOL_AVAILABLE>());
    BOOST_CHECK_EQUAL(1, pool.getInfo().get<POOL_BUSY>());
    BOOST_CHECK(c1.isConnected() and c1.idle());
    
    RawClient& c2 = pool.acquire();
    BOOST_CHECK(pool.invariant());
    SIZE++; // auto incremented
    BOOST_CHECK_EQUAL(SIZE, pool.getInfo().get<POOL_SIZE>());
    BOOST_CHECK_EQUAL(SIZE - 2, pool.getInfo().get<POOL_AVAILABLE>());
    BOOST_CHECK_EQUAL(2, pool.getInfo().get<POOL_BUSY>());
    BOOST_CHECK(c2.isConnected() and c2.idle());

    RawClient& c3 = pool.acquire();
    BOOST_CHECK(pool.invariant());
    SIZE++; // auto increment
    BOOST_CHECK_EQUAL(SIZE, pool.getInfo().get<POOL_SIZE>());
    BOOST_CHECK_EQUAL(SIZE - 3, pool.getInfo().get<POOL_AVAILABLE>());
    BOOST_CHECK_EQUAL(3, pool.getInfo().get<POOL_BUSY>());
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
        BOOST_CHECK_EQUAL(SIZE, pool.getInfo().get<POOL_SIZE>());
        BOOST_CHECK_EQUAL(SIZE - 3, pool.getInfo().get<POOL_AVAILABLE>());
        BOOST_CHECK_EQUAL(3, pool.getInfo().get<POOL_BUSY>());
    }
    
    c3.getResponse();
    BOOST_CHECK_EQUAL(RawClient::Idle, c3.getStatus());
    
    // release
    
    pool.release();
    BOOST_CHECK(pool.invariant());
    BOOST_CHECK_EQUAL(SIZE, pool.getInfo().get<POOL_SIZE>());
    BOOST_CHECK_EQUAL(SIZE - 2, pool.getInfo().get<POOL_AVAILABLE>());
    BOOST_CHECK_EQUAL(2, pool.getInfo().get<POOL_BUSY>());
    
    pool.release();
    BOOST_CHECK(pool.invariant());
    BOOST_CHECK_EQUAL(SIZE, pool.getInfo().get<POOL_SIZE>());
    BOOST_CHECK_EQUAL(SIZE - 1, pool.getInfo().get<POOL_AVAILABLE>());
    BOOST_CHECK_EQUAL(1, pool.getInfo().get<POOL_BUSY>());
    
    pool.release();
    BOOST_CHECK(pool.invariant());
    BOOST_CHECK_EQUAL(SIZE, pool.getInfo().get<POOL_SIZE>());
    BOOST_CHECK_EQUAL(SIZE, pool.getInfo().get<POOL_AVAILABLE>());
    BOOST_CHECK_EQUAL(0, pool.getInfo().get<POOL_BUSY>());
}

struct Worker {
    Worker(ConnectionPool& p) : pool(p) {}
    void work(int i) {
        RawClient& c = pool.acquire();
        BOOST_CHECK(pool.invariant());
        
        c.sendRequest(i, getMessage(i));
        int sleepTime = (std::rand() % 5) + 1;
        DLOG(INFO) << "worker " << i << " sleeping for " << sleepTime << " secs";
        boost::this_thread::sleep(boost::posix_time::seconds(sleepTime));
        c.getResponse();
        
        pool.release();
        BOOST_CHECK(pool.invariant());
        
        DLOG(INFO) << "worker " << i << " done";
    }
private:    
    ConnectionPool& pool;
};
    
    

BOOST_FIXTURE_TEST_CASE(concurrency_test, AsioService) {
    using boost::thread;

    const size_t NUM_THREADS = 5;
    size_t SIZE = 1;
    
    ConnectionPool pool(service, iterator, SIZE, true, NUM_THREADS);
    BOOST_CHECK(pool.invariant());
    
    Worker w(pool);
    
    boost::ptr_vector<thread> threads;
    for (size_t i = 0; i < NUM_THREADS; ++i) {
        threads.push_back(new thread(&Worker::work, &w, i+1));
    }
    
    for (size_t i = 0; i < NUM_THREADS; ++i) {
        threads[i].join();
    }
}

#endif
