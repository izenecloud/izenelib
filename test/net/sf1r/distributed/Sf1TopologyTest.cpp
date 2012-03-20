/* 
 * File:   Sf1TopologyTest.cpp
 * Author: Paolo D'Apice
 * 
 * Created on February 22, 2012, 10:06 AM
 */

#define BOOST_TEST_MODULE Sf1TopologyTest
#include <boost/test/unit_test.hpp>

#include "../common.h"
#include "net/sf1r/distributed/Sf1Topology.hpp"
#include <boost/foreach.hpp>
#include <iostream>

using namespace NS_IZENELIB_SF1R;
using std::string;
using std::set;
using std::vector;
using std::cout;
using std::endl;


BOOST_AUTO_TEST_SUITE(Sf1TopologyTest)


/* enable/disable dump to stdout */
//#define DUMP

void 
dumpNodes(NodeContainer& nodes) {
#ifdef DUMP
    cout << "=== dump nodes ===" << endl;
    BOOST_FOREACH(Sf1Node node, nodes) {
        cout << node << endl;
    }
    cout << "=== end ===" << endl;
#endif
}

void 
dumpCollections(const set<string>& index, NodeCollectionsContainer& colls) {
#ifdef DUMP    
    cout << "--- dump collections ---" << endl;
    BOOST_FOREACH(string col, index) {
        NodeCollectionsRange range = colls.equal_range(col);
        cout << col << " =>";
        for (NodeCollectionsContainer::iterator it = range.first; it != range.second; ++it) {
            cout << " " << it->second->getPath();
        }
        cout << endl;
    }
    cout << "--- end ---" << endl;
#endif
}


/** Operations example. */
BOOST_AUTO_TEST_CASE(operations_test) {
    Sf1Topology topology;
    BOOST_CHECK_EQUAL(0, topology.getCollectionIndex().size());
    
    // add
    
    topology.addNode("/test/test", "collection#coll1$dataport#18121$baport#18181$masterport#18131$host#host");
    BOOST_CHECK_EQUAL(1, topology.count());
    BOOST_CHECK_EQUAL(1, topology.getCollectionIndex().size());
    BOOST_CHECK_EQUAL(1, topology.getCollectionIndex().count("coll1"));
    BOOST_CHECK(not topology.isPresent("/test/node"));
#ifdef ENABLE_ZK_TEST
    dumpCollections(topology.getCollectionIndex(), topology._colls());   
#endif
    BOOST_CHECK_EQUAL(1, topology.count("coll1"));
    BOOST_CHECK_EQUAL(0, topology.count("coll2"));
#ifdef ENABLE_ZK_TEST
    dumpNodes(topology._nodes());
#endif
    
    topology.addNode("/test/node", "collection#coll1,coll2$dataport#18121$baport#18181$masterport#18131$host#host");
    BOOST_CHECK_EQUAL(2, topology.count());
    BOOST_CHECK_EQUAL(2, topology.getCollectionIndex().size());
    BOOST_CHECK_EQUAL(1, topology.getCollectionIndex().count("coll1"));
    BOOST_CHECK_EQUAL(1, topology.getCollectionIndex().count("coll2"));
    BOOST_CHECK(topology.isPresent("/test/node"));
#ifdef ENABLE_ZK_TEST
    dumpCollections(topology.getCollectionIndex(), topology._colls());
#endif
    BOOST_CHECK_EQUAL(2, topology.count("coll1"));
    BOOST_CHECK_EQUAL(1, topology.count("coll2"));
#ifdef ENABLE_ZK_TEST
    dumpNodes(topology._nodes());
#endif
    
    // update
    
    topology.updateNode("/test/node", "collection#coll2$dataport#18121$baport#18181$masterport#18131$host#host");
    BOOST_CHECK_EQUAL(2, topology.count());
    BOOST_CHECK_EQUAL(2, topology.getCollectionIndex().size());
    BOOST_CHECK_EQUAL(1, topology.getCollectionIndex().count("coll1"));
    BOOST_CHECK_EQUAL(1, topology.getCollectionIndex().count("coll2"));
    BOOST_CHECK(topology.isPresent("/test/node"));
#ifdef ENABLE_ZK_TEST
    dumpCollections(topology.getCollectionIndex(), topology._colls());
#endif
     BOOST_CHECK_EQUAL(1, topology.count("coll1"));
     BOOST_CHECK_EQUAL(1, topology.count("coll2"));
#ifdef ENABLE_ZK_TEST
    dumpNodes(topology._nodes());
#endif
    
    // remove
    
    topology.removeNode("/test/node");
    BOOST_CHECK_EQUAL(1, topology.count());
    BOOST_CHECK_EQUAL(1, topology.getCollectionIndex().size());
    BOOST_CHECK_EQUAL(1, topology.getCollectionIndex().count("coll1"));
    BOOST_CHECK_EQUAL(0, topology.getCollectionIndex().count("coll2"));
    BOOST_CHECK(not topology.isPresent("/test/node"));
#ifdef ENABLE_ZK_TEST
    dumpCollections(topology.getCollectionIndex(), topology._colls());
#endif
    BOOST_CHECK_EQUAL(1, topology.count("coll1"));
    BOOST_CHECK_EQUAL(0, topology.count("coll2"));
#ifdef ENABLE_ZK_TEST
    dumpNodes(topology._nodes());
#endif
}


/** Test fixture. */
struct Nodes {
    Nodes() {
        topology.addNode("/test/node1", "collection#coll1$dataport#18121$baport#18181$masterport#18131$host#host1");
        topology.addNode("/test/node2", "collection#coll2$dataport#18121$baport#18181$masterport#18131$host#host2");
        topology.addNode("/test/node3", "collection#coll1,coll2$dataport#18121$baport#18181$masterport#18131$host#host3");
        topology.addNode("/test/node4", "collection#coll1,coll3$dataport#18121$baport#18181$masterport#18131$host#host4");
#ifdef ENABLE_ZK_TEST
        dumpCollections(topology.getCollectionIndex(), topology._colls());
        dumpNodes(topology._nodes());
#endif
    }
    
    Sf1Topology topology;
};


/** Use node-collections relationship. */
BOOST_FIXTURE_TEST_CASE(nodes_collections_test, Nodes) {
    NodeCollectionsRange range;
    
    BOOST_CHECK_EQUAL(3, topology.count("coll1"));
    range = topology.getNodesFor("coll1");
    BOOST_CHECK(range.first != range.second);
    for (NodeCollectionsIterator it = range.first; it != range.second; ++it) {
        Sf1Node n = it->second;
        BOOST_CHECK((n.getPath() == "/test/node1") xor (n.getPath() == "/test/node3") xor (n.getPath() == "/test/node4"));
    }
    
    // use a list instead of a range, for random access
    NodeCollectionsList list(range.first, range.second);
    BOOST_CHECK_EQUAL(3, list.size());
    BOOST_CHECK(list[0].second.getPath() == "/test/node1");
    BOOST_CHECK(list[1].second.getPath() == "/test/node3");
    BOOST_CHECK(list[2].second.getPath() == "/test/node4");
    
    BOOST_CHECK_EQUAL(2, topology.count("coll2"));
    range = topology.getNodesFor("coll2");
    BOOST_CHECK(range.first != range.second);
    for (NodeCollectionsIterator& it = range.first; it != range.second; ++it) {
        Sf1Node n = it->second;
        BOOST_CHECK((n.getPath() == "/test/node2") xor (n.getPath() == "/test/node3"));
    }
    
    BOOST_CHECK_EQUAL(1, topology.count("coll3"));
    range = topology.getNodesFor("coll3");
    BOOST_CHECK(range.first != range.second);
    for (NodeCollectionsIterator& it = range.first; it != range.second; ++it) {
        Sf1Node n = it->second;
        BOOST_CHECK(n.getPath() == "/test/node4");
    }
    
    BOOST_CHECK_EQUAL(0, topology.count("coll5"));
    range = topology.getNodesFor("coll5");
    BOOST_CHECK(range.first == range.second);
}


/** Use nodes. */
BOOST_FIXTURE_TEST_CASE(all_nodes_test, Nodes) {
    NodeListRange range = topology.getNodes();
    BOOST_CHECK_EQUAL(4, range.second - range.first);
}


/** Access a node by path/position. */
BOOST_FIXTURE_TEST_CASE(node_path_test, Nodes) {
    const Sf1Node& node = topology.getNodeAt("/test/node1");
    BOOST_CHECK_EQUAL("/test/node1", node.getPath());
    BOOST_CHECK_EQUAL("host1", node.getHost());
    
    const Sf1Node& same = topology.getNodeAt(0);
    BOOST_CHECK_EQUAL(node, same);
}

BOOST_AUTO_TEST_SUITE_END()
