/* 
 * File:   ZooKeeperRouter.hpp
 * Author: Paolo D'Apice
 *
 * Created on February 17, 2012, 2:32 PM
 */

#ifndef ZOOKEEPERROUTER_HPP
#define	ZOOKEEPERROUTER_HPP

#include "../config.h"
#include "Sf1Node.hpp"
#include <3rdparty/zookeeper/ZooKeeper.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <map>
#include <string>
#include <vector>


NS_IZENELIB_SF1R_BEGIN

namespace iz = izenelib::zookeeper;

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
     * @param recvTimeout TODO
     */
    ZooKeeperRouter(const std::string& hosts, const int recvTimeout);
    
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
     * @return The list of all known SF1 instances.
     */
    std::vector<Sf1Node>* getSf1Nodes() const;
    
    /**
     * @param collection The collection to be searched.
     * @return The list of all known SF1 instances hosting the given collection.
     */
    std::vector<Sf1Node>* getSf1Nodes(const std::string& collection) const;
    
    /**
     * The watcher need to access private member functions.
     */
    friend class Sf1Watcher;
    
private:
    
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
    iz::ZooKeeper* client;
    
    /// ZooKeeper event handler.
    Sf1Watcher* watcher;
    
    /// Actual SF1 instances.
    Sf1Topology* topology;
};

NS_IZENELIB_SF1R_END

#endif	/* ZOOKEEPERROUTER_HPP */
