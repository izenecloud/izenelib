/* 
 * File:   ZooKeeperRouter.cpp
 * Author: Paolo D'Apice
 * 
 * Created on February 17, 2012, 2:32 PM
 */

#include "net/sf1r/distributed/ZooKeeperRouter.hpp"
#include "net/sf1r/distributed/Sf1Topology.hpp"
#include "../PoolFactory.hpp"
#include "../RawClient.hpp"
#include "RoundRobinPolicy.hpp"
#include "Sf1Watcher.hpp"
#include "ZooKeeperNamespace.hpp"
#include "util/kv2string.h"
#include <boost/foreach.hpp>
#include <boost/regex.hpp>
#include <glog/logging.h>


NS_IZENELIB_SF1R_BEGIN

using iz::ZooKeeper;
using std::string;
using std::vector;


namespace {

const bool AUTO_RECONNECT = true;
typedef vector<string> strvector;

}


ZooKeeperRouter::ZooKeeperRouter(PoolFactory* poolFactory, 
        const string& hosts, int timeout) : factory(poolFactory) {
    // 0. connect to ZooKeeper
    LOG(INFO) << "Connecting to ZooKeeper servers: " << hosts;
    client.reset(new iz::ZooKeeper(hosts, timeout, AUTO_RECONNECT));
    if (not client->isConnected()) {
        // XXX: this should be done in the ZooKeeper client
        throw iz::ZooKeeperException("Not connected to (servers): " + hosts);
    }
    
    // 1. register a watcher that will control the Sf1Driver instances
    LOG(INFO) << "Registering watcher ...";
    watcher.reset(new Sf1Watcher(*this));
    client->registerEventHandler(watcher.get());
    
    // 2. obtain the list of actually running SF1 servers
    LOG(INFO) << "Getting actual topology ...";
    topology.reset(new Sf1Topology);
    loadTopology();
    
    // 3. set the routing policy
    LOG(INFO) << "Routing policy ...";
    policy.reset(new RoundRobinPolicy(*topology));
    
    LOG(INFO) << "ZooKeeperRouter ready";
}


ZooKeeperRouter::~ZooKeeperRouter() {
    BOOST_FOREACH(PoolContainer::value_type& i, pools) {
        delete i.second;
    }
    
    DLOG(INFO) << "ZooKeeperRouter destroyed";
}


void
ZooKeeperRouter::loadTopology() {
    strvector clusters;
    client->getZNodeChildren(ROOT_NODE, clusters, ZooKeeper::WATCH);
    
    BOOST_FOREACH(const string& s, clusters) {
        DLOG(INFO) << "found: " << s;
        string cluster = s + TOPOLOGY;
        addClusterNode(cluster);
    }
    LOG(INFO) << "found (" << topology->count() << ") nodes";
}


void
ZooKeeperRouter::addClusterNode(const string& cluster) {
    if (client->isZNodeExists(cluster, ZooKeeper::WATCH)) {
        DLOG(INFO) << "cluster: " << cluster;
        
        strvector replicas;
        client->getZNodeChildren(cluster, replicas, ZooKeeper::WATCH);

        BOOST_FOREACH(const string& replica, replicas) {
            DLOG(INFO) << "replica: " << replica;
            strvector nodes;
            client->getZNodeChildren(replica, nodes, ZooKeeper::WATCH);

            BOOST_FOREACH(const string& path, nodes) {
                addSf1Node(path);
            }
        }
    }
}


void
ZooKeeperRouter::addSf1Node(const string& path) {
    boost::lock_guard<boost::mutex> lock(mutex);

    LOG(INFO) << "sf1 node: " << path;
    
    if (topology->isPresent(path)) return;
    
    string data;
    client->getZNodeData(path, data, ZooKeeper::WATCH);
    LOG(INFO) << "node data: " << data;

    // add node into topology
    topology->addNode(path, data);

    // add pool for the node
    if (factory != NULL) { // Should be NULL only in tests
        const Sf1Node& node = topology->getNodeAt(path);
        DLOG(INFO) << "getting connection pool for node: " << node.getPath();
        ConnectionPool* pool = factory->newConnectionPool(node);
        pools.insert(PoolContainer::value_type(node.getPath(), pool));
    }
}


void 
ZooKeeperRouter::updateNodeData(const std::string& path) {
    boost::lock_guard<boost::mutex> lock(mutex);

    string data;
    client->getZNodeData(path, data, iz::ZooKeeper::WATCH);
    LOG(INFO) << "node data: " << data;
    
    topology->updateNode(path, data);
}


void
ZooKeeperRouter::removeClusterNode(const string& path) {
    boost::lock_guard<boost::mutex> lock(mutex);
    
    DLOG(INFO) << "cluster: " << path;
    
    // remove node from topology
    topology->removeNode(path);
    
    // remove its pool
    ConnectionPool* pool = pools[path];
    CHECK_EQ(1, pools.erase(path));
    delete pool;
}


void
ZooKeeperRouter::watchChildren(const std::string& path) {
    strvector children;
    client->getZNodeChildren(path, children, ZooKeeper::WATCH);
    DLOG(INFO) << "Watching children of: " << path;
    BOOST_FOREACH(const string& s, children) {
        DLOG(INFO) << "child: " << s;
        if (boost::regex_match(s, NODE_REGEX)) {
            addSf1Node(s);
        } else {
            if (boost::regex_search(s, TPLG_REGEX)) {
                watchChildren(s);
            }
        }
    }
}

#ifdef ENABLE_ZK_TEST
NodeListRange
ZooKeeperRouter::getSf1Nodes() const {
    return topology->getNodes();
}


NodeCollectionsRange
ZooKeeperRouter::getSf1Nodes(const string& collection) const {
    return topology->getNodesFor(collection);
}
#endif

const Sf1Node& 
ZooKeeperRouter::resolve(const std::string collection) const {
    if (collection.empty()) {
        DLOG(INFO) << "No collection specified, resolving to all nodes ...";
        
        if (topology->count() == 0) {
            LOG(WARNING) << "No routes, throwing RoutingError";
            throw RoutingError();
        }
        
        // choose a node according to the routing policy
        return policy->getNode();
    } else {
        DLOG(INFO) << "Resolving nodes for collection: " << collection << " ...";
        
        if (topology->count(collection) == 0) {
            LOG(WARNING) << "No routes for collection: " << collection 
                         << ", throwing RoutingError";
            throw RoutingError(collection);
        }
        
        // choose a node according to the routing policy
        return policy->getNodeFor(collection);
    }
}


RawClient&
ZooKeeperRouter::getConnection(const string& collection) throw (RoutingError) {
    const Sf1Node& node = resolve(collection);
    DLOG(INFO) << "Resolved to node: " << node.getPath();
    
    // get a connection from the node
    ConnectionPool* pool = pools[node.getPath()];
    CHECK(pool) << "NULL pool";
    return pool->acquire();
}


void 
ZooKeeperRouter::releaseConnection(const RawClient& connection) {
    pools[connection.getPath()]->release();
}
    

NS_IZENELIB_SF1R_END
