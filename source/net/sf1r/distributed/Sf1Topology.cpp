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


void
Sf1Topology::addNode(const string& path, const string& data) {
    DLOG(INFO) << "adding node: " << path << " ...";
    
    Sf1Node node(path, data);
    // add node
    CHECK(nodes.insert(node).second) << "node "<< path << " not added";
    // add collections
    BOOST_FOREACH(string collection, node.getCollections()) {
        index.insert(collection);
        collections.insert(CollectionsContainer::value_type(collection, &*nodes.find(path)));
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
    NodePathIterator node = nodes.find(path);
    BOOST_FOREACH(string col, node->getCollections()) {
        size_t n = collections.count(col);
        if (n == 1) { // only one node handling that collection
            collections.erase(col);
            index.erase(col);
            
            break;
        }
        
        CollectionsRange range = collections.equal_range(col);
        for (CollectionsIterator it = range.first; it != range.second; ++it) {
            if (it->second->getPath() == path) {
                collections.erase(it);
            }
        }
    }
    
    // remove node
    CHECK(nodes.erase(path) == 1) << "node "<< path << " not removed";
}


NodePathIterator
Sf1Topology::getNodeAt(const std::string& path) {
    return nodes.find(path);
}


NodeList
Sf1Topology::getNodesFor(const string& collection) {
    NodeList list;
    
    CollectionsRange range = collections.equal_range(collection);
    for (CollectionsIterator it = range.first; it != range.second; ++it) {
        list.push_back(it->second);
    }
    
    return list;
}


NodeListRange 
Sf1Topology::getNodes() {
    NodeListIndex& index = nodes.get<list>();
    return std::make_pair(index.begin(), index.end());
}

        
NS_IZENELIB_SF1R_END
