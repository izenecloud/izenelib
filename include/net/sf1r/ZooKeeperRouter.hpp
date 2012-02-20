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
#include <string>
#include <vector>

NS_IZENELIB_SF1R_BEGIN

namespace iz = izenelib::zookeeper;

typedef std::vector<Sf1Node> Sf1List;



/**
 * ZooKeeper proxy class, acting as a router to actual SF1 instances.
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
     * @return the list of all known SF1 instances
     */
    std::vector<Sf1Node> getSf1List() const {
        return sf1List;
    }
    
private:
    
    /**
     * Get the list of running SF1 instances.
     */
    void setSf1List();
    
    /// ZooKeeper client
    iz::ZooKeeper client;
    /// List of actual SF1 instances.
    Sf1List sf1List;
};

NS_IZENELIB_SF1R_END

#endif	/* ZOOKEEPERROUTER_HPP */

