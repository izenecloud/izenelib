/* 
 * File:   ZooKeeperRouter.hpp
 * Author: Paolo D'Apice
 *
 * Created on February 17, 2012, 2:32 PM
 */

#ifndef ZOOKEEPERROUTER_HPP
#define	ZOOKEEPERROUTER_HPP

#include "../config.h"
#include "../Errors.hpp"
#include "NodeContainer.hpp"
#include "PoolContainer.hpp"
#include "Sf1Node.hpp"
#include "3rdparty/zookeeper/ZooKeeper.hpp"
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <map>
#include <string>
#include <vector>


NS_IZENELIB_SF1R_BEGIN

namespace iz = izenelib::zookeeper;

class PoolFactory;
class RawClient;
class RoutingPolicy;
class Sf1Watcher;
class Sf1Topology;


/**
 * ZooKeeper proxy class, acting as a router to the actual SF1 instances.
 * It contains a mapping of all the running SF1 nodes as a \ref Sf1Topology.
 * The \ref Sf1Watcher class is the event handler managing changes in
 * the actual topology.
 */
class ZooKeeperRouter : private boost::noncopyable {
public:
    
    /**
     * Constructor.
     * @param hosts A list of ZooKeeper hosts in the format "host:port[,host:port]".
     * @param timeout Session timeout.
     * @throw ZooKeeperException if cannot connect to ZooKeeper.
     */
    ZooKeeperRouter(PoolFactory* poolFactory,
            const std::string& hosts, const int timeout, const std::string& match_master_name);
    
    /**
     * Destructor.
     */
    ~ZooKeeperRouter();
    
    /**
     * @return true if connected to ZooKeeper, false otherwise.
     */
    bool isConnected() /*const*/ { 
        return client->isConnected();
    }

    /**
     * Get a connection to a node hosting the given collection.
     * @param collection The collection name.
     * @return A reference to the RawClient.
     * @throw RoutingError if no route is found.
     */
    RawClient& getConnection(const std::string& collection);
    
    /**
     * Release a connection.
     * @param connection The connection to be released.
     */
    void releaseConnection(const RawClient& connection);
    
    /**
     * Get a connection to all nodes hosting the given collection.
     * @param collection The collection name.
     * @return A list of references to the RawClient.
     * @throw RoutingError if no route is found.
     */
    std::vector<RawClient*> getConnections(const std::string& collection);
    
    /**
     * Release connections.
     * @param connections The connections to be released.
     */
    void releaseConnections(const std::vector<RawClient*> connections);
    
    
    /**
     * Get all the nodes from the actual topology.
     * @return A range of nodes.
     */
    NodeListRange getSf1Nodes() const;
    
    /**
     * Get all the nodes hosting the given collection from the actual topology.
     * @param collection The collection name
     * @return A range of nodes.
     */
    NodeCollectionsRange getSf1Nodes(const std::string& collection) const;
 
private:
    
    /** The watcher need to access private member functions. */
    friend class Sf1Watcher;
    
    /** Adds a search topology. */
    void addSearchTopology(const std::string& path);
    
    /** Add a new SF1 node in the topology. */
    void addSf1Node(const std::string& path);
    
    /** Updates the SF1 node information. */
    void updateNodeData(const std::string& path);
    
    /** Removes an existing SF1 node. */
    void removeSf1Node(const std::string& path);
    
    /** Watch a node for its children's changes. */
    void watchChildren(const std::string& path);
    
    /** Get the actual SF1 topology (called during initialization). */
    void loadTopology();
    
    /** Resolve to a node in the topology. */
    const Sf1Node& resolve(const std::string collection) const;
    
private:
    
    /// Mutex for topology changes.
    boost::mutex mutex;
    /// Condition variable for connection pool deletion.
    boost::condition_variable condition;
    
    /// ZooKeeper client.
    boost::scoped_ptr<iz::ZooKeeper> client;
    
    /// ZooKeeper event handler.
    boost::scoped_ptr<Sf1Watcher> watcher;
    
    /// Actual SF1 instances.
    boost::scoped_ptr<Sf1Topology> topology;
    
    /// Routing policy.
    boost::scoped_ptr<RoutingPolicy> policy;
    
    /// ConnectionPool factory;
    PoolFactory* factory;
    
    /// Connection pools.
    PoolContainer pools;
    
    std::string match_master_name_;
};

NS_IZENELIB_SF1R_END

#endif	/* ZOOKEEPERROUTER_HPP */
