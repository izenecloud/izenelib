/* 
 * File:   Sf1Topology.cpp
 * Author: Paolo D'Apice
 * 
 * Created on February 22, 2012, 11:17 AM
 */

#include "net/sf1r/distributed/Sf1Topology.hpp"
#include <boost/foreach.hpp>
#include <glog/logging.h>


NS_IZENELIB_SF1R_BEGIN
     
using std::string;
using std::vector;


Sf1Topology::Sf1Topology() {
    DLOG(INFO) << "Topology initialized";
}


Sf1Topology::~Sf1Topology() {
    DLOG(INFO) << "Topology destroyed";
}


void
Sf1Topology::addNode(const string& path, const string& data) {
    DLOG(INFO) << "adding node: " << path << " ...";
    
    Sf1Node node(path, data);
    // add node
    CHECK(nodes.insert(node).second) << "node "<< path << " not added";
    // add collections
    BOOST_FOREACH(string collection, node.getCollections()) {
        index.insert(collection);
        collections.insert(NodeCollectionsContainer::value_type(collection, &*nodes.find(path)));
    }
}


void
Sf1Topology::updateNode(const string& path, const string& data) {
    DLOG(INFO) << "updating node: " << path << "...";
    
    // quick & dirty (TM)
    removeNode(path);
    addNode(path, data);
}


void
Sf1Topology::removeNode(const string& path) {
    DLOG(INFO) << "removing node: " << path << "...";
    
    // remove collections
    BOOST_FOREACH(string col, nodes.find(path)->getCollections()) {
        size_t n = collections.count(col);
        if (n == 1) { // only one node handling that collection
            collections.erase(col);
            index.erase(col);
            
            break;
        }
        
        NodeCollectionsRange range = collections.equal_range(col);
        for (NodeCollectionsIterator it = range.first; it != range.second; ++it) {
            if (it->second->getPath() == path) {
                collections.erase(it);
                }
            }
        }
    
    // remove node
    CHECK(nodes.erase(path) == 1) << "node "<< path << " not removed";
    changed();
}


const Sf1Node&
Sf1Topology::getNodeAt(const std::string& path) {
    return *nodes.find(path);
}


const Sf1Node&
Sf1Topology::getNodeAt(const size_t& pos) {
    return nodes.get<list>()[pos];
}


NodeCollectionsRange
Sf1Topology::getNodesFor(const string& collection) {
    return collections.equal_range(collection);
}


NodeListRange 
Sf1Topology::getNodes() {
    NodeListIndex& index = nodes.get<list>();
    return std::make_pair(index.begin(), index.end());
}

        
NS_IZENELIB_SF1R_END
