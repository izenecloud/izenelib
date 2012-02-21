/* 
 * File:   ZooKeeperRouter.hpp
 * Author: Paolo D'Apice
 *
 * Created on February 17, 2012, 2:32 PM
 */

#ifndef ZOOKEEPERROUTER_HPP
#define	ZOOKEEPERROUTER_HPP

#include "config.h"
#include "types.h"
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


/**
 * Actual SF1 cluster topology.
 * Each SF1 node is accessible through its ZooKeeper path.
 */
typedef std::map<std::string, Sf1Node> Sf1Topology;


/**
 * A list view on the \ref Sf1Topology.
 */
typedef std::vector<Sf1Node> Sf1List;


/**
 * ZooKeeper proxy class, acting as a router to actual SF1 instances.
 * It contains a map of all the running SF1 nodes as a \ref Sf1Topology.
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
        return client.isConnected();
    }
    
    /**
     * @return the list of all known SF1 instances.
     */
    Sf1List getSf1List() const;
    
    /**
     * The watcher need to access private member functions.
     */
    friend class Sf1Watcher;
    
private:
    
    /** Updates the SF1 node information. */
    void updateNodeData(const std::string& path);
    
    /** Add a new SF1 node. */
    void addClusterNode(const std::string& path);
    
    /** Removes an existing SF1 node. */
    void removeClusterNode(const std::string& path);
    
private:
    
    boost::mutex mutex;
    
    /**
     * Get the list of running SF1 instances.
     * Called during initialization.
     */
    void setSf1List();
    
    /// ZooKeeper client
    iz::ZooKeeper client;
    
    /// actual SF1 instances.
    Sf1Topology topology;
    
    /// Watcher
    Sf1Watcher* watcher;
};

NS_IZENELIB_SF1R_END

#endif	/* ZOOKEEPERROUTER_HPP */

