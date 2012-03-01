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


/** Test the Sf1Node class. */
BOOST_AUTO_TEST_SUITE(Sf1NodeTest)

/** Equivalence of Sf1Node's constructors. */
BOOST_AUTO_TEST_CASE(constructor_test) {
    Sf1Node n1("/test/path", "host#localhost$baport#1024$collection#aaa,bbb");
    Sf1Node n2("/test/path", "localhost", 1024, "aaa,bbb");
    Sf1Node n3("/test/another", "localhost", 1024, "aaa,bbb");
    
    BOOST_CHECK_EQUAL(n1, n2);
    BOOST_CHECK_NE(n1, n3);
}

BOOST_AUTO_TEST_SUITE_END()


/** Test the multi-indexed container. */
BOOST_AUTO_TEST_SUITE(NodesContainerTest)

/** Test fixture. */
struct Nodes {
    Nodes() {
        container.insert(Sf1Node("/test/node1", "host1", 18181, "coll1"));
        container.insert(Sf1Node("/test/node2", "host2", 18181, "coll2"));
        container.insert(Sf1Node("/test/node3", "host3", 18181, "coll1,coll2"));
        container.insert(Sf1Node("/test/node4", "host4", 18181, "dummy"));
    }
    
    NodesContainer container;
};

/** Access using different indices. */
BOOST_FIXTURE_TEST_CASE(access_test, Nodes) {
    // default (path)
    BOOST_CHECK_EQUAL(1, container.count("/test/node1"));
    BOOST_CHECK_EQUAL(1, container.count("/test/node2"));
    BOOST_CHECK_EQUAL(1, container.count("/test/node3"));
    BOOST_CHECK_EQUAL(1, container.count("/test/node4"));
    BOOST_CHECK_EQUAL("host1", container.find("/test/node1")->getHost());
    
    // by path (explicit)
    NodesPathIndex& path_index = container.get<path>();
    BOOST_CHECK_EQUAL(1, path_index.count("/test/node1"));
    BOOST_CHECK_EQUAL(1, path_index.count("/test/node2"));
    BOOST_CHECK_EQUAL(1, path_index.count("/test/node3"));
    BOOST_CHECK_EQUAL(1, path_index.count("/test/node4"));
    BOOST_CHECK_EQUAL("host1", path_index.find("/test/node1")->getHost());
    
    // random access (explicit)
    NodesListIndex& list_index = container.get<list>();
    BOOST_CHECK_EQUAL(4, list_index.size());
    BOOST_CHECK_EQUAL("/test/node1", list_index[0].getPath());
    BOOST_CHECK_EQUAL("/test/node2", list_index[1].getPath());
    BOOST_CHECK_EQUAL("/test/node3", list_index[2].getPath());
    BOOST_CHECK_EQUAL("/test/node4", list_index[3].getPath());
}

/** Delete using different indices. */
BOOST_FIXTURE_TEST_CASE(erase_path_test, Nodes) {
    // default (path)
    container.erase("/test/node4");
    BOOST_CHECK_EQUAL(0, container.count("/test/node4"));
    
    // by path (explicit)
    NodesPathIndex& path_index =  container.get<path>();
    path_index.erase("/test/node3");
    BOOST_CHECK_EQUAL(0, container.count("/test/node3"));
}

BOOST_AUTO_TEST_SUITE_END()


/** Test the wrapper class Sf1Topology. */
BOOST_AUTO_TEST_SUITE(Sf1TopologyTest)

void
checkCollections(Sf1Topology& topology, const std::string& collection,
                 const size_t expectedSize) {
    vector<Sf1Node> nodes;
    topology.getNodes(nodes, collection);
    BOOST_CHECK_EQUAL(expectedSize, nodes.size());
}

/* enable/disable dump to stdout */
//#define DUMP

void 
dumpNodes(NodesContainer& nodes) {
#ifdef DUMP
    cout << "=== dump nodes ===" << endl;
    BOOST_FOREACH(Sf1Node node, nodes) {
        cout << node << endl;
    }
    cout << "=== end ===" << endl;
#endif
}

void 
dumpCollections(set<string>& index, CollectionsContainer& colls) {
#ifdef DUMP    
    cout << "--- dump collections ---" << endl;
    BOOST_FOREACH(string col, index) {
        pair<CollectionsContainer::iterator, CollectionsContainer::iterator> range
                = colls.equal_range(col);
        cout << col << " =>";
        for (CollectionsContainer::iterator it = range.first; it != range.second; ++it) {
            cout << " " << it->second;
        }
        cout << endl;
    }
    cout << "--- end ---" << endl;
#endif
}

/** Operations example. */
BOOST_AUTO_TEST_CASE(operations_test) {
    Sf1Topology topology;
    
    // add
    
    topology.addNode("/test/test", "collection#coll1$dataport#18121$baport#18181$masterport#18131$host#host");
    BOOST_CHECK_EQUAL(1, topology.count());
    BOOST_CHECK(not topology.isPresent("/test/node"));
#ifdef ENABLE_ZK_TEST
    dumpCollections(topology._index(), topology._colls());   
#endif
    checkCollections(topology, "coll1", 1);
    checkCollections(topology, "coll2", 0);
#ifdef ENABLE_ZK_TEST
    dumpNodes(topology._nodes());
#endif
    
    topology.addNode("/test/node", "collection#coll1,coll2$dataport#18121$baport#18181$masterport#18131$host#host");
    BOOST_CHECK_EQUAL(2, topology.count());
    BOOST_CHECK(topology.isPresent("/test/node"));
#ifdef ENABLE_ZK_TEST
    dumpCollections(topology._index(), topology._colls());
#endif
    checkCollections(topology, "coll1", 2);
    checkCollections(topology, "coll2", 1);
#ifdef ENABLE_ZK_TEST
    dumpNodes(topology._nodes());
#endif
    
    // update
    
    topology.updateNode("/test/node", "collection#coll2$dataport#18121$baport#18181$masterport#18131$host#host");
    BOOST_CHECK_EQUAL(2, topology.count());
    BOOST_CHECK(topology.isPresent("/test/node"));
#ifdef ENABLE_ZK_TEST
    dumpCollections(topology._index(), topology._colls());
#endif
    checkCollections(topology, "coll1", 1);
    checkCollections(topology, "coll2", 1);
#ifdef ENABLE_ZK_TEST
    dumpNodes(topology._nodes());
#endif
    
    // remove
    
    topology.removeNode("/test/node");
    BOOST_CHECK_EQUAL(1, topology.count());
    BOOST_CHECK(not topology.isPresent("/test/node"));
#ifdef ENABLE_ZK_TEST
    dumpCollections(topology._index(), topology._colls());
#endif
    checkCollections(topology, "coll1", 1);
    checkCollections(topology, "coll2", 0);
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
        dumpCollections(topology._index(), topology._colls());
        dumpNodes(topology._nodes());
#endif
    }
    
    Sf1Topology topology;
};

/** Use node-collections relationship. */
BOOST_FIXTURE_TEST_CASE(nodes_collections_test, Nodes) {
    vector<Sf1Node> nodes;
    
    topology.getNodes(nodes, "coll1");
    BOOST_CHECK_EQUAL(3, nodes.size());
    BOOST_FOREACH(Sf1Node n, nodes) {
        BOOST_CHECK(n.getPath() == "/test/node1" or n.getPath() == "/test/node3" or n.getPath() == "/test/node4");
    }
    
    topology.getNodes(nodes, "coll2");
    BOOST_CHECK_EQUAL(2, nodes.size());
    BOOST_FOREACH(Sf1Node n, nodes) {
        BOOST_CHECK(n.getPath() == "/test/node2" or n.getPath() == "/test/node3");
    }
    
    topology.getNodes(nodes, "coll3");
    BOOST_CHECK_EQUAL(1, nodes.size());
    BOOST_FOREACH(Sf1Node n, nodes) {
        BOOST_CHECK(n.getPath() == "/test/node4");
    }
    
    topology.getNodes(nodes, "coll5");
    BOOST_CHECK_EQUAL(0, nodes.size());
}

/** Use nodes. */
BOOST_FIXTURE_TEST_CASE(all_collections_test, Nodes) {
    vector<Sf1Node> nodes;
    topology.getNodes(nodes);
    BOOST_CHECK_EQUAL(4, nodes.size());
}

BOOST_AUTO_TEST_SUITE_END()
