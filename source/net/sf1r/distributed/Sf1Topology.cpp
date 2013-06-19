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
Sf1Topology::addNode(const string& path, const string& data, bool emit) {
    DLOG(INFO) << "adding node: " << path << " ...";
    
    Sf1Node node(path, data);
    // add node
    CHECK(nodes.insert(node).second) << "node "<< path << " not added";
    // add collections
    BOOST_FOREACH(const string& collection, node.getCollections()) {
        collectionsIndex.insert(collection);
        nodeCollections.insert(NodeCollectionsContainer::value_type(collection, *nodes.find(path)));
    }
    
    if (emit) {
        changed();
    }
}


void
Sf1Topology::updateNode(const string& path, const string& data, bool emit) {
    DLOG(INFO) << "updating node: " << path << "...";
    
    // quick & dirty (TM)
    removeNode(path, false);
    addNode(path, data, false);
    
    if (emit) {
        changed();
    }
}

void Sf1Topology::clearNodes()
{
    nodeCollections.clear();
    collectionsIndex.clear();
    nodes.clear();
    changed();
}

void
Sf1Topology::removeNode(const string& path, bool emit) {
    DLOG(INFO) << "removing node: " << path << "...";
    
    if (nodes.find(path) == nodes.end())
        return;
    // remove collections
    BOOST_FOREACH(const string& col, nodes.find(path)->getCollections()) {
        if (nodeCollections.count(col) == 1) { // only one node handling that collection
            nodeCollections.erase(col);
            collectionsIndex.erase(col);
            
            continue;
        }
        
        NodeCollectionsRange range = nodeCollections.equal_range(col);
        NodeCollectionsIterator it = range.first;
        while ( it != range.second) {
            if (it->second.getPath() == path) {
                nodeCollections.erase(it);
                //LOG(INFO) << "remove from sf1topology : " << it->second.getPath() << ", in col:" << col;
                range = nodeCollections.equal_range(col);
                it = range.first;
            }
            else
                ++it;
        }
    }
    
    // remove node
    CHECK(nodes.erase(path) == 1) << "node "<< path << " not removed";
    if (emit) {
        changed();
    }
}


const Sf1Node&
Sf1Topology::getNodeAt(const std::string& path) {
    if (nodes.find(path) == nodes.end())
        throw "node for path not exist!";
    return *nodes.find(path);
}


const Sf1Node&
Sf1Topology::getNodeAt(const size_t& pos) {
    if (pos >= nodes.get<list>().size())
        throw "out of range.";
    return nodes.get<list>()[pos];
}


NodeCollectionsRange
Sf1Topology::getNodesFor(const string& collection) {
    return nodeCollections.equal_range(collection);
}


NodeListRange 
Sf1Topology::getNodes() {
    NodeListIndex& index = nodes.get<list>();
    return std::make_pair(index.begin(), index.end());
}

        
NS_IZENELIB_SF1R_END
