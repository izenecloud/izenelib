/* 
 * File:   RoundRobinPolicy.cpp
 * Author: paolo
 * 
 * Created on March 5, 2012, 2:59 PM
 */

#include "net/sf1r/distributed/Sf1Topology.hpp"
#include "RoundRobinPolicy.hpp"
#include <boost/signals2.hpp>
#include <glog/logging.h>

NS_IZENELIB_SF1R_BEGIN

using std::string;


RoundRobinPolicy::RoundRobinPolicy(Sf1Topology &topology)
        : RoutingPolicy(topology), counter(0) {
    updateCollections();
    topology.changed.connect(boost::bind(&RoundRobinPolicy::resetCounter, this));
    topology.changed.connect(boost::bind(&RoundRobinPolicy::updateCollections, this));
    DLOG(INFO) << "initialized";
}


RoundRobinPolicy::~RoundRobinPolicy() {
    DLOG(INFO) << "destroyed";
}


void
RoundRobinPolicy::resetCounter() {
    DLOG(INFO) << "resetting counter due to topology changes";
    counter = 0;
}


void
RoundRobinPolicy::updateCollections() {
    DLOG(INFO) << "updating collections map due to topology changes";
    collections.clear();
    
    BOOST_FOREACH(const string& collection, topology.getCollectionIndex()) {
        NodeCollectionsRange range = topology.getNodesFor(collection);
        string k(collection);
        collections.insert(k, new NodeCollectionsList(range.first, range.second));
        ccounter[collection] = 0;
    }
}


const Sf1Node& 
RoundRobinPolicy::getNode() {
    size_t index = counter++ % topology.count();
    DLOG(INFO) << "index = " << index;
    
    return topology.getNodeAt(index);
}


const Sf1Node& 
RoundRobinPolicy::getNodeFor(const std::string collection) {
    const NodeCollectionsList& list = collections.at(collection);
    size_t index = ccounter[collection]++ % list.size();
    DLOG(INFO) << "index = " << index;
    
    return *(list[index].second);
}
    
    
NS_IZENELIB_SF1R_END
