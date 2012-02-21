/* 
 * File:   ZookeeperRouterTest.cpp
 * Author: Paolo D'Apice
 * 
 * Created on February 17, 2012, 2:06 PM
 */

#define BOOST_TEST_MODULE ZookeeperRouterTest
#include <boost/test/unit_test.hpp>

#include "common.h"
#include "3rdparty/zookeeper/ZooKeeper.hpp"
#include "net/sf1r/ZooKeeperRouter.hpp"
#include <boost/thread.hpp>

using namespace izenelib::zookeeper;
using namespace NS_IZENELIB_SF1R;
using namespace std;

BOOST_AUTO_TEST_CASE(dummy) {
    BOOST_TEST_MESSAGE("dummy empty test");
    cout << "For this test you need an SF1 running on you machine." << endl;
}

/*
 * Example of use
 * ===
 * 
 * Test for new node:
 * 1. start this test executable
 * 2. start a new SF1 which connects to the ZooKeeper hosts specified below
 * 3. check post = pre + 1
 * 
 */

#ifdef ENABLE_ZK_TEST


void printList(const Sf1List& list) {
    if (list.empty()) {
        cout << "*** no SF1 ***" << endl;
        return;
    }
        
    BOOST_FOREACH(Sf1Node n, list) {
        cout << "* " << n << endl;
        BOOST_CHECK(not n.getCollections().empty());
    }
}


void sleep(const int& time) {
    cout << "Sleeping for " << time << " seconds (so that you can start/stop your SF1) ..." << endl;
    for (int i = 0; i < time; i++) {
        //cout << "." << flush;
        boost::this_thread::sleep(boost::posix_time::seconds(1));
    }
    cout << " Awaked!" << endl;
}


BOOST_AUTO_TEST_CASE(sanity) {
    string hosts = "localhost:2181";
            //"180.153.140.110:2181,180.153.140.111:2181,180.153.140.112:2181";
    int recvTimeout = 2000;
    
    int interval = 20;
    
    // out router, defining watchers on SF1 nodes
    ZooKeeperRouter router(hosts, recvTimeout);
    BOOST_CHECK(router.isConnected());
    
    Sf1List pre = router.getSf1List();
    printList(pre);
    
    sleep(interval); // here you should start a local SF1
    
    Sf1List post = router.getSf1List();
    printList(post);
    BOOST_CHECK_EQUAL(pre.size() + 1, post.size());
    
    vector<string> coll = post.front().getCollections();
    sleep(interval); // here you should change the data of your SF1 with zkCli
                     // $ set /SF1R-XXX/SearchTopology/Replica1/Node1 
                     //   collection#dummy$dataport#18121$baport#18181$masterport#18131$host#localhost
    
    post = router.getSf1List();\
    BOOST_CHECK(coll.size() != post.front().getCollections().size());
    
    sleep(interval); // here you should stop your local SF1
    
    post = router.getSf1List();
    printList(post);
    BOOST_CHECK_EQUAL(pre.size(), post.size());
    
    // FIXME: on destruction, serverside there is the following exception
    // Unexpected Exception: java.nio.channels.CancelledKeyException
}

#endif
