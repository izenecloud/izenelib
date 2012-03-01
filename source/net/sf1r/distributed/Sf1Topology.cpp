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
    DLOG(INFO) << "adding node: " << path << "...";
    
    Sf1Node node(path, data);
    // add node
    CHECK(nodes.insert(node).second) << "node "<< path << " not added";
    // add collections
    BOOST_FOREACH(string collection, node.getCollections()) {
        index.insert(collection);
        collections.insert(CollectionsContainer::value_type(collection, node.getPath()));
    }
}


void
Sf1Topology::updateNode(const string& path, const string& data) {
    DLOG(INFO) << "updating node: " << path << "...";
    
    // XXX tricky: remove and add
    removeNode(path);
    addNode(path, data);
}


void
Sf1Topology::removeNode(const string& path) {
    DLOG(INFO) << "removing node: " << path << "...";
    
    // remove collections
    NodesPathIterator node = nodes.find(path);
    BOOST_FOREACH(string col, node->getCollections()) {
        size_t n = collections.count(col);
        if (n == 1) { // only one node handling that collection
            collections.erase(col);
            index.erase(col);
            
            break;
        }
        
        CollectionsRange range = collections.equal_range(col);
        for (CollectionsIterator it = range.first; it != range.second; ++it) {
            if (it->second == path) {
                collections.erase(it);
            }
        }
    }
    
    // remove node
    CHECK(nodes.erase(path) == 1) << "node "<< path << " not removed";
}


inline void
ensureEmpty(vector<Sf1Node>& nn) {
    if (not nn.empty()) {
        nn.clear();
    }
}

void
Sf1Topology::getNodes(vector<Sf1Node>& nn, const string& collection) {
    ensureEmpty(nn);
    
    CollectionsRange range = collections.equal_range(collection);
    for (CollectionsIterator it = range.first; it != range.second; ++it) {
        string node = it->second;
        nn.push_back(*(nodes.find(node)));
    }
    
    // TODO: add here a routing policy (e.g. round robin)
}


void 
Sf1Topology::getNodes(vector<Sf1Node>& nn) {
    ensureEmpty(nn);
    
    NodesContainer::index<list>::type& index = nodes.get<list>();
    NodesContainer::index<list>::type::iterator it = index.begin();
    for(; it != index.end(); ++it) {
        nn.push_back(*it);
    }
    
    // TODO: add here a routing policy (e.g. round robin)
}

        
NS_IZENELIB_SF1R_END
