/* 
 * File:   ZooKeeperRouter.cpp
 * Author: Paolo D'Apice
 * 
 * Created on February 17, 2012, 2:32 PM
 */

#include "net/sf1r/ZooKeeperRouter.hpp"
#include "Sf1Watcher.hpp"
#include "ZooKeeperNamespace.hpp"
#include "util/kv2string.h"
#include <boost/foreach.hpp>
#include <glog/logging.h>


NS_IZENELIB_SF1R_BEGIN

using iz::ZooKeeper;
using izenelib::util::kv2string;
using std::string;
using std::vector;


namespace {
    const bool AUTO_RECONNECT = true;
    typedef vector<string> strvector;
}


ZooKeeperRouter::ZooKeeperRouter(const string& hosts, int recvTimeout) 
        : client(hosts, recvTimeout, AUTO_RECONNECT) {
    // 1. register a watcher that will control the Sf1Driver instances
    LOG(INFO) << "Registering watcher ...";
    watcher = new Sf1Watcher(this);
    client.registerEventHandler(watcher);
    
    // 2. obtain the list of actually running SF1 servers
    LOG(INFO) << "Getting actual topology ...";
    setSf1List();
    
    // 3. connect to each of them using the Sf1Driver class
    // TODO
}


ZooKeeperRouter::~ZooKeeperRouter() {
    // XXX deleting the watcher while it is still used by the client?
    delete watcher;
}


Sf1List
ZooKeeperRouter::getSf1List() const {
    Sf1List list;
    BOOST_FOREACH(Sf1Topology::value_type pair, topology) {
        list.push_back(pair.second);
    }
    return list;
}


void 
ZooKeeperRouter::updateNodeData(const std::string& path) {
    boost::lock_guard<boost::mutex> lock(mutex);

    string data;
    client.getZNodeData(path, data, ZooKeeper::WATCH);

    Sf1Topology::iterator it = topology.find(path);
    CHECK(it != topology.end()) << "Node " << path << " not found!";
    
    it->second.update(data);
}


void
ZooKeeperRouter::addClusterNode(const string& cluster) {
    if (client.isZNodeExists(cluster, ZooKeeper::WATCH)) {
        DLOG(INFO) << "cluster: " << cluster;
        
        strvector replicas;
        client.getZNodeChildren(cluster, replicas, ZooKeeper::WATCH);

        BOOST_FOREACH(string replica, replicas) {
            DLOG(INFO) << "replica: " << replica;
            strvector nodes;
            client.getZNodeChildren(replica, nodes, ZooKeeper::WATCH);

            BOOST_FOREACH(string n, nodes) {
                boost::lock_guard<boost::mutex> lock(mutex);

                string data;
                client.getZNodeData(n, data, ZooKeeper::WATCH);

                Sf1Node node(n, data);
                LOG(INFO) << "node: " << node;

                topology.insert(Sf1Topology::value_type(n, node));
            }
        }
    }
}


void
ZooKeeperRouter::removeClusterNode(const string& cluster) {
    boost::lock_guard<boost::mutex> lock(mutex);
    
    DLOG(INFO) << "cluster: " << cluster;
    topology.erase(cluster);
}


void
ZooKeeperRouter::setSf1List() {
    strvector clusters;
    //client.isZNodeExists(ROOT_NODE, ZooKeeper::WATCH);
    client.getZNodeChildren(ROOT_NODE, clusters, ZooKeeper::WATCH);
    
    BOOST_FOREACH(string s, clusters) {
        string cluster = s + SEARCH_TOPOLOGY;
        addClusterNode(cluster);
    }
}

NS_IZENELIB_SF1R_END
