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
#include <util/scheduler.h>
#include "../Utils.hpp"

NS_IZENELIB_SF1R_BEGIN

using iz::ZooKeeper;
using std::string;
using std::vector;
using namespace izenelib::util;

namespace {

const bool AUTO_RECONNECT = true;
typedef vector<string> strvector;

}


ZooKeeperRouter::ZooKeeperRouter(PoolFactory* poolFactory, 
        const string& hosts, int timeout, const std::string& match_master_name,
        int set_seq, int total_set_num)
    : factory(poolFactory), match_master_name_(match_master_name),
    set_seq_(set_seq), total_set_num_(total_set_num)
{
    assert(set_seq_ <= total_set_num_);
    // 0. connect to ZooKeeper
    LOG(INFO) << "Connecting to ZooKeeper servers: " << hosts;
    client.reset(new iz::ZooKeeper(hosts, timeout, AUTO_RECONNECT));

    while (not client->isConnected()) {
        timeout -= 100;
        if (timeout < 0)
        {
            // XXX: this should be done in the ZooKeeper client
            throw iz::ZooKeeperException("Not connected to (servers): " + hosts);
        }
        usleep(100*1000);
    }
    
    // 1. register a watcher that will control the Sf1Driver instances
    LOG(INFO) << "Registering watcher ...";
    watcher.reset(new Sf1Watcher(*this));
    client->registerEventHandler(watcher.get());
    
    // 2. obtain the list of actually running SF1 servers
    LOG(INFO) << "Getting actual topology ...";
    topology.reset(new Sf1Topology);
    backup_topology.reset(new Sf1Topology);
    loadTopology();
    
    // 3. set the routing policy
    LOG(INFO) << "Routing policy ...";
    policy.reset(new RoundRobinPolicy(*topology, *backup_topology));
    
    LOG(INFO) << "ZooKeeperRouter ready";
    Scheduler::addJob("UpdateNodeDataOnTimer", 30*1000, 30*1000,
        boost::bind(&ZooKeeperRouter::updateNodeDataOnTimer, this, _1));
}

void ZooKeeperRouter::reconnect()
{
    LOG(INFO) << "reconnecting....";
    if (!client)
        return;
    client->disconnect();
    client->connect();
    int timeout = 10*1000;
    while(not client->isConnected())
    {
        timeout -= 100;
        if (timeout < 0)
        {
            // XXX: this should be done in the ZooKeeper client
            throw iz::ZooKeeperException("Re-connect failed " );
        }
        usleep(100*1000);
    }
    // clear old nodes.
    clearSf1Nodes();

    loadTopology();
}

ZooKeeperRouter::~ZooKeeperRouter() {

    {
        WriteLockT rwlock(shared_mutex);
        std::vector<std::string> tmp_list;
        BOOST_FOREACH(PoolContainer::value_type& i, pools) {
            tmp_list.push_back(i.first);
        }
        
        for (size_t i = 0; i < tmp_list.size(); ++i)
        {
            removeNodeFromPools(tmp_list[i], rwlock);
        }
    }

    Scheduler::removeJob("UpdateNodeDataOnTimer", true);

    LOG(INFO) << "ZooKeeperRouter closed";
}

void ZooKeeperRouter::reloadTopology()
{
    if(client && client->isConnected())
    {
        // clear old nodes.
        clearSf1Nodes();
        loadTopology();
    }
}

void
ZooKeeperRouter::loadTopology() {
    strvector clusters;
    client->getZNodeChildren(ROOT_NODE, clusters, ZooKeeper::WATCH);
    
    LOG(INFO) << "Scanning root node ...";
    BOOST_FOREACH(const string& s, clusters) {
        if (boost::regex_match(s, SF1R_ROOT_REGEX)) { // this is a SF1 node
            LOG(INFO) << "node: " << s;
            addSearchTopology(s + TOPOLOGY);
        }
        else
            LOG(INFO) << "not sf1r node : " << s;
    }
    LOG(INFO) << "found (" << topology->count() << ") active nodes";
    LOG(INFO) << "found (" << backup_topology->count() << ") backup active nodes";
}


void
ZooKeeperRouter::addSearchTopology(const string& searchTopology) {
    if (client->isZNodeExists(searchTopology, ZooKeeper::WATCH)) {
        LOG(INFO) << "found search topology: " << searchTopology;
        
        DLOG(INFO) << "Searching for replicas ...";
        strvector replicas;
        client->getZNodeChildren(searchTopology, replicas, ZooKeeper::WATCH);

        if (replicas.empty()) {
            DLOG(INFO) << "no replica found";
            return;
        }
        
        BOOST_FOREACH(const string& replica, replicas) {

            addSf1Node(replica);

            strvector nodes;
            client->getZNodeChildren(replica, nodes, ZooKeeper::WATCH);

            if (nodes.empty()) {
                continue;
            }
            BOOST_FOREACH(const string& path, nodes) {
                addSf1Node(path);
            }
        }
    }
}


bool
ZooKeeperRouter::addSf1Node(const string& path) {
    UpgradeLockT lock(shared_mutex);

    LOG(INFO) << "adding SF1 node: [" << path << "]";
    
    if (topology->isPresent(path)) {
        LOG(INFO) << "node already exists, skipping";
        return true;
    }
    if (backup_topology->isPresent(path)) {
        LOG(INFO) << "node already exists, skipping";
        return true;
    }
    
    string data;
    client->getZNodeData(path, data, ZooKeeper::WATCH);
    LOG(INFO) << "node data: [" << data << "]";

    izenelib::util::kv2string parser;
    parser.loadKvString(data);
    //
    // replicaid and set_seq_ both start from 1.
    Sf1Topology* adding_topology = topology.get();
    int32_t replicaid = parser.getUInt32Value(REPLICAID_KEY);
    if (((replicaid - 1) % total_set_num_) == (set_seq_ - 1))
    {
        LOG(INFO) << "current set_seq_ : " << set_seq_ << " connectting to node in replica : " << replicaid;
    }
    else
    {
        LOG(INFO) << "current set_seq_ : " << set_seq_ << " using as backup node in replica : " << replicaid;
        // adding to backup node.
        adding_topology = backup_topology.get();
    }

    if (not parser.hasKey(MASTERPORT_KEY)) {
        LOG(INFO) << "Not a master node, skipping";
        return false;
    }

    if (!match_master_name_.empty())
    {
        std::string mastername = parser.getStrValue(MASTER_NAME_KEY);
        if(mastername != match_master_name_) { //match special master SF1 node
            LOG(INFO) << "ignore not matched master : " << mastername;
            return false;
        }
    }

    // check for search service
    std::string service_names;
    service_names = parser.getStrValue(SERVICE_NAMES_KEY);
    if (service_names.find(SearchService) == std::string::npos)
    {
        LOG(INFO) << "current node has no search service : " << path;
        return false;
    }

    UpgradeUniqueLockT uniqueLock(lock);
    // add node into topology
    adding_topology->addNode(path, data);

#ifdef ENABLE_ZK_TEST
    if (factory == NULL) { // Should be NULL only in tests
        LOG(WARNING) << "factory is NULL, is this a test?";
        return false;
    }
#endif
    
    // add pool for the node
    const Sf1Node& node = adding_topology->getNodeAt(path);
    DLOG(INFO) << "Getting connection pool for node: " << node.getPath();

    try
    {
        ConnectionPool* pool = factory->newConnectionPool(node);
        pools.insert(PoolContainer::value_type(node.getPath(), pool));
    }
    catch(...)
    {
        LOG(ERROR) << "connect failed.";
        // remove node from topology
        adding_topology->removeNode(path);
        return false;
    }

    return true;
}


void ZooKeeperRouter::updateNodeDataOnTimer(int calltype)
{
    bool need_rescan = false;
    {
        ReadLockT lock(shared_mutex);
        if (topology->count() == 0) {
            LOG(INFO) << "Empty topology, re-scan topology...";
            need_rescan = true;
        }
    }
    if (need_rescan)
        loadTopology();

    WriteLockT lock(shared_mutex);
    LOG(INFO) << "updating SF1 node in timer callback, total nodes : " << topology->count();
    if (policy)
        policy->updateSlowCounterForAll();

    WaitingMapT::iterator it = waiting_update_path_.begin();
    while(it != waiting_update_path_.end())
    {
        if (it->second > 0 && topology->isPresent(it->first))
        {
            // update
            LOG(INFO) << "updating SF1 node in timer : [" << it->first << "]";
            string data;
            client->getZNodeData(it->first, data, iz::ZooKeeper::WATCH);
            LOG(INFO) << "node data: [" << data << "]";
            topology->updateNode(it->first, data);
            it->second = 0;
        }
        if (it->second > 0 && backup_topology->isPresent(it->first))
        {
            // update
            LOG(INFO) << "updating SF1 node in timer : [" << it->first << "]";
            string data;
            client->getZNodeData(it->first, data, iz::ZooKeeper::WATCH);
            LOG(INFO) << "node data: [" << data << "]";
            backup_topology->updateNode(it->first, data);
            it->second = 0;
        }
        ++it;
    }
}

void 
ZooKeeperRouter::updateNodeData(const string& path) {
    string data;
    bool force_update = false;
    Sf1Topology* updating_topology = topology.get();
    {
	    ReadLockT lock(shared_mutex);

	    if (not topology->isPresent(path)) {
		    LOG(INFO) << "Node not in topology, skipping";
            if (backup_topology->isPresent(path))
            {
                updating_topology = backup_topology.get();
            }
            else
                return;
	    }
	    client->getZNodeData(path, data, iz::ZooKeeper::WATCH);
        // check if collection changed.
        izenelib::util::kv2string parser;
        parser.loadKvString(data);
        std::string new_coll = parser.getStrValue(SearchService + COLLECTION_KEY);
        std::vector<std::string> collections;
        split(new_coll, DELIMITER_CHAR, collections);
        if (collections != updating_topology->getNodeAt(path).getCollections())
        {
            LOG(INFO) << "collection changed in node, need update immediatly." << path;
            force_update = true;
        }
        if (updating_topology->getNodeAt(path).getServiceState() != parser.getStrValue(SERVICE_STATE_KEY))
        {
            LOG(INFO) << "service state changed in node, need update immediatly." << path;
            force_update = true;
        }
    }

    WriteLockT rwlock(shared_mutex);
    WaitingMapT::value_type element(path, 1);
    std::pair<WaitingMapT::iterator, bool> insert_wait = waiting_update_path_.insert(element);
    WaitingMapT::mapped_type& cur_wait_info = insert_wait.first->second;
    if (!insert_wait.second)
    {
        cur_wait_info += 1;
    }
    if (data.empty())
    {
        cur_wait_info = 0;
        LOG(INFO) << "update node data is empty, remove it." << path;
        // remove its pool
        removeNodeFromPools(path, rwlock);
        // remove node from topology
        updating_topology->removeNode(path);

#ifdef ENABLE_ZK_TEST
        if (factory == NULL) { // Should be NULL only in tests
            LOG(WARNING) << "factory is NULL, is this a test?";
            return;
        }
#endif
    }
    else
    {
        if (!force_update && cur_wait_info < 100)
        {
            cur_wait_info++;
            return;
        }
	    LOG(INFO) << "updating SF1 node: [" << path << "]";
	    LOG(INFO) << "node data: [" << data << "]";
        updating_topology->updateNode(path, data);
        cur_wait_info = 0;
    }
}

void ZooKeeperRouter::clearSf1Nodes() {
    WriteLockT rwlock(shared_mutex);
    LOG(INFO) << "clearing SF1 node";
    waiting_update_path_.clear();
    std::vector<std::string> tmp_list;
    PoolContainer::iterator it = pools.begin();
    while (it != pools.end())
    {
        tmp_list.push_back(it->first);
        ++it;
    }
    for (size_t i = 0; i < tmp_list.size(); ++i)
        removeNodeFromPools(tmp_list[i], rwlock);

    topology->clearNodes();
    backup_topology->clearNodes();
    LOG(INFO) << "clear nodes finished";
}

void ZooKeeperRouter::increSlowCounter(const std::string& path)
{
    LOG(INFO) << "increasing slow counter for SF1 node: [" << path << "]";
    WriteLockT rwlock(shared_mutex);
    if (policy)
        policy->increSlowCounter(path);
}

void
ZooKeeperRouter::removeSf1Node(const string& path) {
    Sf1Topology* removing_topology = topology.get();
	{
		ReadLockT lock(shared_mutex);

		if (not topology->isPresent(path)) {
			LOG(INFO) << "Node not in topology, skipping";
            if (not backup_topology->isPresent(path))
                return;
            removing_topology = backup_topology.get();
		}
	}
    
    LOG(INFO) << "removing SF1 node: [" << path << "]";
    
    WriteLockT rwlock(shared_mutex);

    removeNodeFromPools(path, rwlock);
    // remove node from topology
    if (removing_topology->isPresent(path))
        removing_topology->removeNode(path);

    waiting_update_path_.erase(path);

#ifdef ENABLE_ZK_TEST
    if (factory == NULL) { // Should be NULL only in tests
        LOG(WARNING) << "factory is NULL, is this a test?";
        return;
    }
#endif

}

void ZooKeeperRouter::removeNodeFromPools(const std::string& path, WriteLockT& rwlock)
{
    // remove its pool
    ConnectionPool* pool = NULL;
    while (true)
    {
        PoolContainer::iterator it = pools.find(path);
        if (it == pools.end() || !(it->second))
            return;
        pool = it->second;
        if (pool->isBusy()) {
            LOG(INFO) << "pool is currently in use, waiting : " << path;
            condition.wait(rwlock);
        }
        else
        {
            CHECK_EQ(1, pools.erase(path));
            delete pool;
            return;
        }
    }
}

void
ZooKeeperRouter::watchChildren(const string& path) {
    if (not client->isZNodeExists(path, ZooKeeper::WATCH)) {
        LOG(INFO) << "node [" << path << "] does not exist";
        //boost::smatch what;
        //CHECK(boost::regex_search(path, what, SF1R_ROOT_REGEX));
        //CHECK(not what.empty());
        //
        //DLOG(INFO) << "recurse to [" << what[0] << "] ...";
        //watchChildren(what[0]);
        return;
    }
    
    if (!boost::regex_search(path, TOPOLOGY_REGEX) &&
        !boost::regex_match(path, SF1R_ROOT_REGEX) &&
        path != ROOT_NODE )
    {
        client->isZNodeExists(path, ZooKeeper::NOT_WATCH);
        //LOG(INFO) << "Not Watching children of: " << path;
        return;
    }

    strvector children;
    client->getZNodeChildren(path, children, ZooKeeper::WATCH);
    DLOG(INFO) << "Watching children of: " << path;
    BOOST_FOREACH(const string& s, children) {
        DLOG(INFO) << "child: " << s;
        if (boost::regex_match(s, SF1R_NODE_REGEX)) {
            DLOG(INFO) << "Adding node: " << s;
            addSf1Node(s);
        } else if (boost::regex_search(s, SF1R_ROOT_REGEX)) {
            DLOG(INFO) << "recurse";
            watchChildren(s);
        }
    }
}


//NodeListRange
//ZooKeeperRouter::getSf1Nodes() const {
//    DLOG(INFO) << "Getting all nodes ...";
//    
//    if (topology->count() == 0) {
//        LOG(WARNING) << "Empty topology"; 
//    }
//    return topology->getNodes();
//}
//
//
//NodeCollectionsRange
//ZooKeeperRouter::getSf1Nodes(const string& collection) const {
//    DLOG(INFO) << "Getting all nodes for collection: " << collection << " ...";
//    
//    if (topology->count() == 0) {
//        LOG(WARNING) << "Empty topology"; 
//    }
//    
//    if (topology->count(collection) == 0) {
//        LOG(WARNING) << "No routes for collection: " << collection;
//    }
//    return topology->getNodesFor(collection);
//}


const Sf1Node& 
ZooKeeperRouter::resolve(const string collection) const {
    if (topology->count() == 0 && backup_topology->count() == 0) {
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
            if (backup_topology->count(collection) == 0)
                throw RoutingError("No routes for " + collection);
            LOG(INFO) << "found in backup.";
        }
        
        // choose a node according to the routing policy
        return policy->getNodeFor(collection);
    }
}


RawClient&
ZooKeeperRouter::getConnection(const string& collection) {
    std::string errinfo;
    std::string nodepath;
    {
        ReadLockT lock(shared_mutex);

        const Sf1Node& node = resolve(collection);
        nodepath = node.getPath();
        LOG(INFO) << "Resolved to node: " << nodepath;

        // get a connection from the node
        PoolContainer::const_iterator cit = pools.find(nodepath);
        if (cit == pools.end())
        {
            LOG(INFO) << "no pools for the connection, add new node : " << nodepath;
            {
                LOG(ERROR) << "get connection failed for : " << nodepath;
                throw RoutingError("no ConnectionPool for " + collection);
            }
        }
        ConnectionPool* pool = cit->second;
        CHECK(pool) << "NULL pool";
        try{
            return pool->acquire();
        }catch(NetworkError& e)
        {
            errinfo = e.what();
        }
    }
    LOG(INFO) << "get connection failed for path : " << nodepath;
    if (!client->isZNodeExists(nodepath, ZooKeeper::WATCH))
    {
        removeSf1Node(nodepath);
    }
    throw NetworkError(errinfo);
}


void 
ZooKeeperRouter::releaseConnection(const RawClient& connection) {
    WriteLockT lock(shared_mutex);
    
    DLOG(INFO) << "releasing connection";
    ConnectionPool* pool = pools.find(connection.getPath())->second;
    pool->release(connection); 
    
    if (not pool->isBusy()) {
        DLOG(INFO) << "notifying for condition";
        condition.notify_all();
    }
}


inline void
addConnection(vector<RawClient*>& list, const Sf1Node& node, PoolContainer& pools) {
    ConnectionPool* pool = pools.find(node.getPath())->second;
    CHECK(pool) << "NULL pool";
    list.push_back(&pool->acquire());
}


vector<RawClient*> 
ZooKeeperRouter::getConnections(const string& collection) {
    ReadLockT lock(shared_mutex);
    Sf1Topology* cur_topology = topology.get();
    if (topology->count() == 0) {
        if (backup_topology->count() == 0)
        {
            LOG(WARNING) << "Empty topology, throwing RoutingError";
            throw RoutingError("Empty topology");
        }
        cur_topology = backup_topology.get();
    }
    
    if (collection.empty()) {
        vector<RawClient*> list;
        NodeListRange nodes = cur_topology->getNodes();
        BOOST_FOREACH(const Sf1Node& node, nodes) {
            addConnection(list, node, pools);
        }
        return list;
    } else {
        if (cur_topology->count(collection) == 0) {
            LOG(WARNING) << "No routes for collection: " << collection 
                         << ", throwing RoutingError";
            throw RoutingError("No routes for " + collection);
        }
        
        vector<RawClient*> list;
        NodeCollectionsRange range = cur_topology->getNodesFor(collection);
        NodeCollectionsList nodes(range.first, range.second);
        BOOST_FOREACH(const NodeCollectionsList::value_type& value, nodes) {
            const Sf1Node& node = value.second;
            addConnection(list, node, pools);
        }
        return list;
    }
}


void 
ZooKeeperRouter::releaseConnections(const vector<RawClient*> connections) {
    BOOST_FOREACH(RawClient* connection, connections) {
        releaseConnection(*connection);
    }
}


NS_IZENELIB_SF1R_END
