/* 
 * File:   Sf1TopologyTest.cpp
 * Author: Paolo D'Apice
 * 
 * Created on February 22, 2012, 10:06 AM
 */

#define BOOST_TEST_MODULE RoundRobinPolicyTest
#include <boost/test/unit_test.hpp>

#include "../common.h"
#include "net/sf1r/distributed/Sf1Topology.hpp"
#include "net/sf1r/distributed/RoundRobinPolicy.hpp"
#include <boost/lexical_cast.hpp>
#include <iostream>

using namespace NS_IZENELIB_SF1R;
using boost::scoped_ptr;
using std::string;
using std::set;
using std::vector;
using std::cout;
using std::endl;


BOOST_AUTO_TEST_SUITE(RoundRobinPolicyTest)

/** Test fixture. */
struct Nodes {
    Nodes() {
        topology.addNode("/test/node1", "collection#coll1$dataport#18121$baport#18181$masterport#18131$host#host1");
        topology.addNode("/test/node2", "collection#coll2$dataport#18121$baport#18181$masterport#18131$host#host2");
        topology.addNode("/test/node3", "collection#coll1,coll2$dataport#18121$baport#18181$masterport#18131$host#host3");
        topology.addNode("/test/node4", "collection#coll1,coll3$dataport#18121$baport#18181$masterport#18131$host#host4");
        
        policy = new RoundRobinPolicy(topology, backup_topology);
    }
    
    ~Nodes() {
        delete policy;
    }

    void testAll(const size_t& stop = -1) {
        for (size_t i = 0; i < 2 * topology.count(); i++) {
            size_t index = i % topology.count() + 1;
            string expected = "/test/node" + boost::lexical_cast<string>(index);
            BOOST_CHECK_EQUAL(expected, policy->getNode().getPath());
            
            if (i == stop) {
                topology.addNode("/test/node5", "collection#coll3$dataport#18121$baport#18181$masterport#18131$host#host5");
                break;
            }
        }
    }
    
    Sf1Topology topology;
    Sf1Topology backup_topology;
    RoutingPolicy* policy;
};


/** Test the round-robin policy on all nodes. */
BOOST_FIXTURE_TEST_CASE(routing_all_nodes_test, Nodes) {
    testAll();
}


/** Test that on topology changes the round-robin index is reset. */
BOOST_FIXTURE_TEST_CASE(routing_all_nodes_changes_test, Nodes) {
    // randomly choose an index on which change the topology
    srand(time(NULL));
    size_t stop = rand() % topology.count();
    testAll(stop);
    testAll();
}


/** Test the round-robin policy on nodes hosting a collection. */
BOOST_FIXTURE_TEST_CASE(routing_collection_test, Nodes) {
    string collection = "coll1";
    BOOST_CHECK_EQUAL("/test/node1", policy->getNodeFor(collection).getPath());
    BOOST_CHECK_EQUAL("/test/node3", policy->getNodeFor(collection).getPath());
    BOOST_CHECK_EQUAL("/test/node4", policy->getNodeFor(collection).getPath());
    BOOST_CHECK_EQUAL("/test/node1", policy->getNodeFor(collection).getPath());
    BOOST_CHECK_EQUAL("/test/node3", policy->getNodeFor(collection).getPath());
    
    collection = "coll2";
    BOOST_CHECK_EQUAL("/test/node2", policy->getNodeFor(collection).getPath());
    BOOST_CHECK_EQUAL("/test/node3", policy->getNodeFor(collection).getPath());
    BOOST_CHECK_EQUAL("/test/node2", policy->getNodeFor(collection).getPath());
    BOOST_CHECK_EQUAL("/test/node3", policy->getNodeFor(collection).getPath());
    BOOST_CHECK_EQUAL("/test/node2", policy->getNodeFor(collection).getPath());
    
    collection = "coll3";
    BOOST_CHECK_EQUAL("/test/node4", policy->getNodeFor(collection).getPath());
    BOOST_CHECK_EQUAL("/test/node4", policy->getNodeFor(collection).getPath());
}


BOOST_AUTO_TEST_SUITE_END()
