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
#include <boost/thread.hpp>

using namespace izenelib::zookeeper;
using namespace NS_IZENELIB_SF1R;
using namespace std;


BOOST_AUTO_TEST_CASE(dummy) {
    BOOST_TEST_MESSAGE("dummy empty test");
    cout << "For this test you need an SF1 running on you machine." << endl;
}


#ifdef ENABLE_ZK_TEST

void printList(const vector<Sf1Node>& list, const string& name = "") {
    if (not name.empty())
        cout << name << ":" << endl;
    
    if (list.empty()) {
        cout << "*** empty ***" << endl;
        return;
    }
        
    BOOST_FOREACH(Sf1Node n, list) {
        cout << "* " << n << endl;
        BOOST_CHECK(not n.getCollections().empty());
    }
}


/** Test fixtures. */
struct ZooKeeperClient {
    ZooKeeperClient() : hosts("localhost:2181"), recvTimeout(2000), 
                        client(hosts, recvTimeout, true) {}
    
    inline string getName(const string& name) {
        return "/SF1R-" + name;
    }
    
    /** Simulates the connection of a new SF1. */
    void addSf1(const string& name) {
        string sf1name = getName(name);
        cout << "adding fake SF1 " << sf1name << " ... ";
        
        
        createNode(sf1name);
        createNode(sf1name + "/SearchTopology");
        createNode(sf1name + "/SearchTopology/Replica1");
        createNode(sf1name + "/SearchTopology/Replica1/Node1", 
                   "collection#foo$dataport#18121$baport#18181$masterport#18131$host#localhost");
        
        cout << "done" << endl;
    }
    
    void change(const string& name) {
        string sf1name = getName(name);
        cout << "changing fake SF1 " << sf1name << " ... ";
        
        client.setZNodeData(sf1name + "/SearchTopology/Replica1/Node1", 
                   "collection#bar$dataport#18121$baport#18181$masterport#18131$host#localhost");
        sleep(1);
        
        cout << "done" << endl;
    }
    
    /** Simulates the disconnection of a new SF1. */
    void removeSf1(const string& name, bool keep = true) {
        string sf1name = getName(name);
        cout << "removing fake SF1 " << sf1name << " ... ";
        
        keep ? removeNode(sf1name + "/SearchTopology") : removeNode(sf1name);
        sleep(1);
        
        cout << "done" << endl;
    }
    
    string hosts;
    int recvTimeout;
    ZooKeeper client;
    
private:
    void createNode(const string& path, const string& data = "") {
        client.createZNode(path, data);
        BOOST_CHECK(client.isZNodeExists(path));
        string tmp;
        client.getZNodeData(path, tmp);
        BOOST_CHECK_EQUAL(data, tmp);
        sleep(1);
    }
    
    void removeNode(const string& path) {
        client.deleteZNode(path, true);
        BOOST_CHECK(not client.isZNodeExists(path));
        sleep(1);
    }
};

void
checkNodes(vector<Sf1Node>* list, const size_t& size) {
    printList(*list, "nodes");
    BOOST_CHECK_EQUAL(size, list->size());
    delete list;
}

void
checkCollections(vector<Sf1Node>* list, const size_t& size, 
        const string& path1 = "", const string& path2 = "") {
    printList(*list, "colls");
    BOOST_CHECK_EQUAL(size, list->size());
    if (not path1.empty())
        BOOST_CHECK_EQUAL(path1, (*list)[0].getPath());
    if (not path2.empty())
        BOOST_CHECK_EQUAL(path2, (*list)[1].getPath());
    delete list;
}

BOOST_FIXTURE_TEST_CASE(topology_test, ZooKeeperClient) {
    BOOST_TEST_CHECKPOINT("Connecting to ZooKeeper ...");
    ZooKeeperRouter router(hosts, recvTimeout);
    if (not router.isConnected()) throw runtime_error("ZooKeeper not found");
    
    checkNodes(router.getSf1Nodes(), 0);
    
    addSf1("fake1");
    checkNodes(router.getSf1Nodes(), 1);
    checkCollections(router.getSf1Nodes("foo"), 1, 
                "/SF1R-fake1/SearchTopology/Replica1/Node1");
    checkCollections(router.getSf1Nodes("bar"), 0);
    
    addSf1("fake2");
    checkNodes(router.getSf1Nodes(), 2);
    checkCollections(router.getSf1Nodes("foo"), 2, 
                "/SF1R-fake1/SearchTopology/Replica1/Node1",
                "/SF1R-fake2/SearchTopology/Replica1/Node1");
    checkCollections(router.getSf1Nodes("bar"), 0);
    
    change("fake2");
    checkNodes(router.getSf1Nodes(), 2);
    checkCollections(router.getSf1Nodes("foo"), 1, 
                "/SF1R-fake1/SearchTopology/Replica1/Node1");
    checkCollections(router.getSf1Nodes("bar"), 1, 
                "/SF1R-fake2/SearchTopology/Replica1/Node1");
    
    removeSf1("fake1", false);
    checkNodes(router.getSf1Nodes(), 1);
    checkCollections(router.getSf1Nodes("foo"), 0);
    checkCollections(router.getSf1Nodes("bar"), 1, 
                "/SF1R-fake2/SearchTopology/Replica1/Node1");
    
    removeSf1("fake2");
    checkNodes(router.getSf1Nodes(), 0);
    checkCollections(router.getSf1Nodes("foo"), 0);
    checkCollections(router.getSf1Nodes("bar"), 0);
    
    // FIXME: on destruction, serverside there is the following exception
    // Unexpected Exception: java.nio.channels.CancelledKeyException
}

#endif
