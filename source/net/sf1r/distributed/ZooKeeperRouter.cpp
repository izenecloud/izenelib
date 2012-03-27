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
    
    LOG(INFO) << "ZooKeeperRouter closed";
}


void
ZooKeeperRouter::loadTopology() {
    strvector clusters;
    client->getZNodeChildren(ROOT_NODE, clusters, ZooKeeper::WATCH);
    
    DLOG(INFO) << "Scanning root node ...";
    BOOST_FOREACH(const string& s, clusters) {
        if (boost::regex_match(s, NODE_REGEX)) { // this is a SF1 node
            DLOG(INFO) << "node: " << s;
            addSearchTopology(s + TOPOLOGY);
        }
    }
    LOG(INFO) << "found (" << topology->count() << ") active nodes";
}


void
ZooKeeperRouter::addSearchTopology(const string& searchTopology) {
    if (client->isZNodeExists(searchTopology, ZooKeeper::WATCH)) {
        DLOG(INFO) << "found search topology: " << searchTopology;
        
        DLOG(INFO) << "Searching for replicas ...";
        strvector replicas;
        client->getZNodeChildren(searchTopology, replicas, ZooKeeper::WATCH);

        if (replicas.empty()) {
            DLOG(INFO) << "no replica found";
            return;
        }
        
        BOOST_FOREACH(const string& replica, replicas) {
            DLOG(INFO) << "replica: " << replica;
            
            DLOG(INFO) << "Searching for nodes ...";
            strvector nodes;
            client->getZNodeChildren(replica, nodes, ZooKeeper::WATCH);

            if (nodes.empty()) {
                DLOG(INFO) << "no node found";
                return;
            }
            
            BOOST_FOREACH(const string& path, nodes) {
                addSf1Node(path);
            }
        }
    }
}


void
ZooKeeperRouter::addSf1Node(const string& path) {
    boost::lock_guard<boost::mutex> lock(mutex);

    LOG(INFO) << "adding SF1 node: [" << path << "]";
    
    if (topology->isPresent(path)) {
        DLOG(INFO) << "node already exists, skipping";
        return;
    }
    
    string data;
    client->getZNodeData(path, data, ZooKeeper::WATCH);
    LOG(INFO) << "node data: [" << data << "]";

    // add node into topology
    topology->addNode(path, data);

#ifdef ENABLE_ZK_TEST
    if (factory == NULL) { // Should be NULL only in tests
        LOG(WARNING) << "factory is NULL, is this a test?";
        return;
    }
#endif
    
    // add pool for the node
    const Sf1Node& node = topology->getNodeAt(path);
    DLOG(INFO) << "Getting connection pool for node: " << node.getPath();
    ConnectionPool* pool = factory->newConnectionPool(node);
    pools.insert(PoolContainer::value_type(node.getPath(), pool));
}


void 
ZooKeeperRouter::updateNodeData(const string& path) {
    boost::lock_guard<boost::mutex> lock(mutex);

    LOG(INFO) << "updating SF1 node: [" << path << "]";
    
    string data;
    client->getZNodeData(path, data, iz::ZooKeeper::WATCH);
    LOG(INFO) << "node data: [" << data << "]";
    
    topology->updateNode(path, data);
}


void
ZooKeeperRouter::removeSf1Node(const string& path) {
    boost::unique_lock<boost::mutex> lock(mutex);
    
    LOG(INFO) << "SF1 node: [" << path << "]";
    
    // remove node from topology
    topology->removeNode(path);

#ifdef ENABLE_ZK_TEST
    if (factory == NULL) { // Should be NULL only in tests
        LOG(WARNING) << "factory is NULL, is this a test?";
        return;
    }
#endif

    // remove its pool
    const PoolContainer::iterator& it = pools.find(path);
    CHECK(pools.end() != it) << "pool not found";
    ConnectionPool* pool = it->second;
    CHECK_NOTNULL(pool);
    
    while (pool->isBusy()) {
        DLOG(INFO) << "pool is currently in use, waiting";
        condition.wait(lock);
    }
    DLOG(INFO) << "removing pool";
    CHECK_EQ(1, pools.erase(path));
    delete pool;
}


void
ZooKeeperRouter::watchChildren(const string& path) {
    if (not client->isZNodeExists(path, ZooKeeper::WATCH)) {
        DLOG(INFO) << "node [" << path << "] does not exist";
        boost::smatch what;
        CHECK(boost::regex_search(path, what, NODE_REGEX));
        CHECK(not what.empty());
        
        DLOG(INFO) << "recurse to [" << what[0] << "] ...";
        watchChildren(what[0]);
        return;
    }
    
    strvector children;
    client->getZNodeChildren(path, children, ZooKeeper::WATCH);
    DLOG(INFO) << "Watching children of: " << path;
    BOOST_FOREACH(const string& s, children) {
        DLOG(INFO) << "child: " << s;
        if (boost::regex_match(s, SEARCH_NODE_REGEX)) {
            DLOG(INFO) << "Adding node: " << s;
            addSf1Node(s);
        } else if (boost::regex_search(s, NODE_REGEX)) {
            DLOG(INFO) << "recurse";
            watchChildren(s);
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
ZooKeeperRouter::resolve(const string collection) const {
    if (topology->count() == 0) {
        LOG(WARNING) << "Empty topology, throwing RoutingError";
        throw RoutingError("Empty topology");
    }
        
    if (collection.empty()) {
        DLOG(INFO) << "No collection specified, resolving to all nodes ...";
        
        // choose a node according to the routing policy
        return policy->getNode();
    } else {
        DLOG(INFO) << "Resolving nodes for collection: " << collection << " ...";
        
        if (topology->count(collection) == 0) {
            LOG(WARNING) << "No routes for collection: " << collection 
                         << ", throwing RoutingError";
            throw RoutingError("No routes for " + collection);
        }
        
        // choose a node according to the routing policy
        return policy->getNodeFor(collection);
    }
}


RawClient&
ZooKeeperRouter::getConnection(const string& collection) {
    boost::lock_guard<boost::mutex> lock(mutex);
    
    const Sf1Node& node = resolve(collection);
    DLOG(INFO) << "Resolved to node: " << node.getPath();
    
    // get a connection from the node
    ConnectionPool* pool = pools.find(node.getPath())->second;
    CHECK(pool) << "NULL pool";
    return pool->acquire();
}


void 
ZooKeeperRouter::releaseConnection(const RawClient& connection) {
    boost::lock_guard<boost::mutex> lock(mutex);
    
    DLOG(INFO) << "releasing connection";
    ConnectionPool* pool = pools.find(connection.getPath())->second;
    pool->release(connection); 
    
    if (not pool->isBusy()) {
        DLOG(INFO) << "notifying for condition";
        condition.notify_one();
    }
}
    

NS_IZENELIB_SF1R_END
