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
#include <boost/foreach.hpp>
#include <vector>

using namespace izenelib::zookeeper;
using namespace NS_IZENELIB_SF1R;
using namespace std;

BOOST_AUTO_TEST_CASE(dummy) {
    BOOST_TEST_MESSAGE("dummy empty test");
}

#ifdef ENABLE_ZK_TEST

BOOST_AUTO_TEST_CASE(sanity) {
    string hosts = "180.153.140.110:2181,180.153.140.111:2181,180.153.140.112:2181";
    int recvTimeout = 2000;
    
    // 'external' client
    ZooKeeper client(hosts, recvTimeout, true);
    BOOST_CHECK(client.isConnected());
    
    // out router, defining watchers on SF1 nodes
    ZooKeeperRouter router(hosts, recvTimeout);
    BOOST_CHECK(router.isConnected());
    
    // get the list of running SF1s
    vector<Sf1Node> preList = router.getSf1List();
    BOOST_CHECK(not preList.empty());
    BOOST_FOREACH(Sf1Node n, preList) {
        cout << n << endl;
        BOOST_CHECK_EQUAL(18181, n.getBaPort());
        BOOST_CHECK(not n.getCollections().empty());
    }
    
#if 0        
    // simulate external change in node topology
    cli.deleteZNode("path/to/sf1", true);
    
    // check te list is changed
    vector<string> postList = router.getSf1List();
    
    BOOST_CHECK_EQUAL(preList.size(), postList.size() + 1);
#endif
    // FIXME: on destruction, serverside there is the following exception
    // Unexpected Exception: java.nio.channels.CancelledKeyException
}

#endif
