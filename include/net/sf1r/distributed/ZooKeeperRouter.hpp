/* 
 * File:   ZooKeeperRouter.hpp
 * Author: Paolo D'Apice
 *
 * Created on February 17, 2012, 2:32 PM
 */

#ifndef ZOOKEEPERROUTER_HPP
#define	ZOOKEEPERROUTER_HPP

#include "../config.h"
#include "NodeContainer.hpp"
#include "PoolContainer.hpp"
#include "Sf1Node.hpp"
#include <3rdparty/zookeeper/ZooKeeper.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <map>
#include <string>
#include <vector>


NS_IZENELIB_SF1R_BEGIN

namespace iz = izenelib::zookeeper;

class PoolFactory;
class RawClient;
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
     * @param timeout Sessio timeout.
     */
    ZooKeeperRouter(PoolFactory* poolFactory,
            const std::string& hosts, const int timeout);
    
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
     * Get a view on all the nodes in the actual topology.
     * @return An iterator range.
     * @see Sf1Topology#getNodes()
     */
    NodeListRange getSf1Nodes() const;
    
    /**
     * Get a view on all the nodes in the actual topology 
     * hosting the specified collection.
     * @param collection The collection name.
     * @return A list of iterators.
     * @see Sf1Topology#getNodesFor()
     */
    NodeList getSf1Nodes(const std::string& collection) const;
    
    /**
     * Get a connection to a node hosting the given collection.
     * @param collection The collection name
     * @return A reference to the RawClient
     */
    RawClient& getConnection(const std::string& collection);
    
    /**
     * Release a connection.
     * @param path The node path for which the collection is released.
     */
    void releaseConnection(const RawClient& connection);
 
private:
    
    /** The watcher need to access private member functions. */
    friend class Sf1Watcher;
    
    /** Adds a new cluster node. */
    void addClusterNode(const std::string& path);
    
    /** Add a new SF1 node in the topology. */
    void addSf1Node(const std::string& path);
    
    /** Updates the SF1 node information. */
    void updateNodeData(const std::string& path);
    
    /** Removes an existing SF1 node. */
    void removeClusterNode(const std::string& path);
    
    /** Watch a node for changes. */
    void watchChildren(const std::string& path);
    
    /** Get the actual SF1 topology (called during initialization) */
    void loadTopology();
    
private:
    
    boost::mutex mutex;
    
    /// ZooKeeper client.
    boost::scoped_ptr<iz::ZooKeeper> client;
    
    /// ZooKeeper event handler.
    boost::scoped_ptr<Sf1Watcher> watcher;
    
    /// Actual SF1 instances.
    boost::scoped_ptr<Sf1Topology> topology;
    
    /// ConnectionPool factory;
    PoolFactory* factory;
    
    /// Connection pools.
    PoolContainer pools;
};

NS_IZENELIB_SF1R_END

#endif	/* ZOOKEEPERROUTER_HPP */
