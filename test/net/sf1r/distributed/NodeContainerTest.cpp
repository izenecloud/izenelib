/* 
 * File:   NodeContainerTest.cpp
 * Author: Paolo D'Apice
 * 
 * Created on March 6, 2012, 12:03 PM
 */

#include <boost/test/unit_test.hpp>

#include "net/sf1r/distributed/NodeContainer.hpp"

using namespace NS_IZENELIB_SF1R;


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
BOOST_AUTO_TEST_SUITE(NodeContainerTest)

/** Test fixture. */
struct Nodes {
    Nodes() {
        container.insert(Sf1Node("/test/node1", "host1", 18181, "coll1"));
        container.insert(Sf1Node("/test/node2", "host2", 18181, "coll2"));
        container.insert(Sf1Node("/test/node3", "host3", 18181, "coll1,coll2"));
        container.insert(Sf1Node("/test/node4", "host4", 18181, "dummy"));
    }
    
    NodeContainer container;
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
    NodePathIndex& path_index = container.get<path>();
    BOOST_CHECK_EQUAL(1, path_index.count("/test/node1"));
    BOOST_CHECK_EQUAL(1, path_index.count("/test/node2"));
    BOOST_CHECK_EQUAL(1, path_index.count("/test/node3"));
    BOOST_CHECK_EQUAL(1, path_index.count("/test/node4"));
    BOOST_CHECK_EQUAL("host1", path_index.find("/test/node1")->getHost());
    
    // random access (explicit)
    NodeListIndex& list_index = container.get<list>();
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
    NodePathIndex& path_index =  container.get<path>();
    path_index.erase("/test/node3");
    BOOST_CHECK_EQUAL(0, container.count("/test/node3"));
}

BOOST_AUTO_TEST_SUITE_END()
