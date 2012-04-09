/* 
 * File:   ZookeeperRouterTest.cpp
 * Author: Paolo D'Apice
 * 
 * Created on February 17, 2012, 2:06 PM
 */

#define BOOST_TEST_MODULE ZookeeperRouterTest
#include <boost/test/unit_test.hpp>

#include "../common.h"
#include "3rdparty/zookeeper/ZooKeeper.hpp"
#include "net/sf1r/distributed/ZooKeeperRouter.hpp"
#include <boost/foreach.hpp>
#include <boost/thread.hpp>

using namespace izenelib::zookeeper;
using namespace NS_IZENELIB_SF1R;
using namespace std;


BOOST_AUTO_TEST_CASE(dummy) {
    cout << "For this test you need ZooKeeper running on you machine." << endl;
}

/*
 * This test requires a running ZooKeeper server.
 */

#ifdef ENABLE_ZK_TEST

const string HOSTS = "localhost:2181";


/** Test fixtures. */
struct ZooKeeperClient {
    ZooKeeperClient() : hosts(HOSTS), recvTimeout(2000), 
                        client(hosts, recvTimeout, true) {}
    
    string getName(const string& name) {
        return "/SF1R-" + name;
    }
    
    /** Simulates the connection of a new SF1. */
    void addSf1(const string& name, bool master = true) {
        string sf1name = getName(name);
        cout << "adding fake SF1 " << sf1name << " ... ";
        
        
        createNode(sf1name);
        createNode(sf1name + "/SearchTopology");
        createNode(sf1name + "/SearchTopology/Replica1");
        createNode(sf1name + "/SearchTopology/Replica1/Node1", 
                   master ? "collection#foo$dataport#18121$baport#18181$masterport#18131$host#localhost"
                          : "collection#foo$dataport#18121$baport#18181$host#localhost");
        
        cout << "done" << endl;
    }
    
    void change(const string& name) {
        string sf1name = getName(name);
        cout << "changing fake SF1 " << sf1name << " ... ";
        
        client.setZNodeData(sf1name + "/SearchTopology/Replica1/Node1", 
                   "collection#bar$dataport#18121$baport#18181$masterport#18131$host#localhost");
        
        cout << "done" << endl;
    }
    
    /** Simulates the disconnection of a new SF1. */
    void removeSf1(const string& name, bool keep = true) {
        string sf1name = getName(name);
        cout << "removing fake SF1 " << sf1name << " ... ";
        
        keep ? removeNode(sf1name + "/SearchTopology") : removeNode(sf1name);
        
        cout << "done" << endl;
    }
    
    const string hosts;
    const int recvTimeout;
    ZooKeeper client;
    
private:
    void createNode(const string& path, const string& data = "") {
        client.createZNode(path, data);
        BOOST_CHECK(client.isZNodeExists(path));
        string tmp;
        client.getZNodeData(path, tmp);
        BOOST_CHECK_EQUAL(data, tmp);
    }
    
    void removeNode(const string& path) {
        client.deleteZNode(path, true);
        BOOST_CHECK(not client.isZNodeExists(path));
    }
};


inline void wait() {
    sleep(2);
}

inline void
printLine(const string& s = "") {
    if (s.empty()) {
        cout << "------------------------------------------------------------------" << endl;
    } else {
        cout << "---------" << s << "---------------------------------------------" << endl;
    }
}

void
printRange(const NodeListRange& range) {
    cout << "nodes:" << endl;
    if (range.second - range.first == 0) {
        cout << "*** empty ***" << endl;
        return;
    }
    for (NodeListIterator it = range.first; it != range.second; ++it) {
        cout << "* " << *it << endl;
        BOOST_CHECK(not it->getCollections().empty());
    }
}

void 
printList(const NodeCollectionsList& list, const string& collection) {
    cout << "nodes[" << collection << "]:" << endl;
    if (list.empty()) {
        cout << "*** empty ***" << endl;
        return;
    }
    
    BOOST_FOREACH(const NodeCollectionsList::value_type& value,  list) {
        Sf1Node n = value.second;
        cout << "* " << n << endl;
        BOOST_CHECK(not n.getCollections().empty());
    }
}

void
checkNodes(const NodeListRange& range, const size_t& size) {
    printRange(range);
    BOOST_CHECK_EQUAL(size, range.second - range.first);
}

void
checkCollections(const string& collection, 
        const ZooKeeperRouter& router, const size_t& size, 
        const string& path1 = "", const string& path2 = "") {
    NodeCollectionsRange range = router.getSf1Nodes(collection);
    NodeCollectionsList list(range.first, range.second);
    printList(list, collection);
    BOOST_CHECK_EQUAL(size, list.size());
    if (not path1.empty()) {
        if (list.size() > 0) {
            BOOST_CHECK_EQUAL(path1, list[0].second.getPath());
        } else {
            BOOST_ERROR("list size mismatch");
        }
    }
    if (not path2.empty()) {
        if (list.size() > 1) {
            BOOST_CHECK_EQUAL(path2, list[1].second.getPath());
        } else {
            BOOST_ERROR("list size mismatch");
        }
    }
}

BOOST_FIXTURE_TEST_CASE(topology_test, ZooKeeperClient) {
    ZooKeeperRouter router(NULL, hosts, recvTimeout);
    if (not router.isConnected()) throw runtime_error("ZooKeeper not found");
    
    const int numNodes = router.getSf1Nodes().second - router.getSf1Nodes().first;
    checkNodes(router.getSf1Nodes(), numNodes);
    printLine();
    
    printLine("adding fake1");
    addSf1("fake1");
    wait();
    checkNodes(router.getSf1Nodes(), numNodes + 1);
    checkCollections("foo", router, 1, 
                "/SF1R-fake1/SearchTopology/Replica1/Node1");
    checkCollections("bar", router, 0);
    printLine();
    
    printLine("adding fake2");
    addSf1("fake2");
    wait();
    checkNodes(router.getSf1Nodes(), numNodes + 2);
    checkCollections("foo", router, 2, 
                "/SF1R-fake1/SearchTopology/Replica1/Node1",
                "/SF1R-fake2/SearchTopology/Replica1/Node1");
    checkCollections("bar", router, 0);
    printLine();
    
    printLine("adding fake3 no master");
    addSf1("fake3", false);
    wait();
    checkNodes(router.getSf1Nodes(), numNodes + 2);
    checkCollections("foo", router, 2, 
                "/SF1R-fake1/SearchTopology/Replica1/Node1",
                "/SF1R-fake2/SearchTopology/Replica1/Node1");
    checkCollections("bar", router, 0);
    printLine();
    
    printLine("changing fake2");
    change("fake2");
    wait();
    checkNodes(router.getSf1Nodes(), numNodes + 2);
    checkCollections("foo", router, 1, 
                "/SF1R-fake1/SearchTopology/Replica1/Node1");
    checkCollections("bar", router, 1, 
                "/SF1R-fake2/SearchTopology/Replica1/Node1");
    printLine();
    
    printLine("removing fake1");
    removeSf1("fake1");
    wait();
    checkNodes(router.getSf1Nodes(), numNodes + 1);
    checkCollections("foo", router, 0);
    checkCollections("bar", router, 1, 
                "/SF1R-fake2/SearchTopology/Replica1/Node1");
    printLine();
     
    printLine("removing fake2");
    removeSf1("fake2");
    wait();
    checkNodes(router.getSf1Nodes(), numNodes);
    checkCollections("foo", router, 0);
    checkCollections("bar", router, 0);
    printLine();
    
    printLine("re-adding fake1");
    addSf1("fake1");
    wait();
    checkNodes(router.getSf1Nodes(), numNodes + 1);
    checkCollections("foo", router, 1, 
                "/SF1R-fake1/SearchTopology/Replica1/Node1");
    checkCollections("bar", router, 0);
    printLine();
    
    printLine("re-adding fake2");
    addSf1("fake2");
    wait();
    checkNodes(router.getSf1Nodes(), numNodes + 2);
    checkCollections("foo", router, 2, 
                "/SF1R-fake1/SearchTopology/Replica1/Node1",
                "/SF1R-fake2/SearchTopology/Replica1/Node1");
    checkCollections("bar", router, 0);
    printLine();
    
    printLine("re-removing fake1");
    removeSf1("fake1");
    wait();
    checkNodes(router.getSf1Nodes(), numNodes + 1);
    checkCollections("bar", router, 0);
    checkCollections("foo", router, 1, 
                "/SF1R-fake2/SearchTopology/Replica1/Node1");
    printLine();
    
    printLine("re-removing fake2");
    removeSf1("fake2");
    wait();
    checkNodes(router.getSf1Nodes(), numNodes);
    checkCollections("foo", router, 0);
    checkCollections("bar", router, 0);
    
    printLine("removing fake3");
    removeSf1("fake3");
    wait();
    checkNodes(router.getSf1Nodes(), numNodes);
    checkCollections("foo", router, 0);
    checkCollections("bar", router, 0);
    
    printLine("finished");
    checkNodes(router.getSf1Nodes(), numNodes);
    printLine();
    
    // XXX: on destruction, serverside there is the following exception
    // Unexpected Exception: java.nio.channels.CancelledKeyException
}

#endif
