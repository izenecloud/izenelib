/* 
 * File:   Sf1Node.hpp
 * Author: Paolo D'Apice
 *
 * Created on February 20, 2012, 10:35 AM
 */

#ifndef SF1NODE_HPP
#define	SF1NODE_HPP

#include "../config.h"
#include <boost/foreach.hpp>
#include <ostream>
#include <vector>
#include <string>

NS_IZENELIB_SF1R_BEGIN


/**
 * Data structure representing a running SF1 instance.
 */
class Sf1Node {
public:
    
    /**
     * Constructor.
     * @param path
     * @param data
     */
    Sf1Node(const std::string& path, const std::string& data);
    
    /**
     * Constructor.
     * @param path
     * @param host
     * @param port
     * @param collections
     */
    Sf1Node(const std::string& path, const std::string& host,
            const uint32_t port, const std::string& collections);
    
    ~Sf1Node() {}
    
    /**
     * Updates the node information.
     */
    void update(const std::string& data);
    
    /**
     * @return The ZooKeeper path of this node.
     */
    std::string getPath() const {
        return path;
    }
    
    /**
     * @return The hostname of this node.
     */
    std::string getHost() const {
        return host;
    }
    
    /**
     * @return The port on which the SF1 accepts requests on this node.
     */
    uint32_t getPort() const {
        return port;
    }
    
    const std::string& getServiceState() const {
        return service_state;
    }
   /**
     * @return The collections associated to this node.
     */
    std::vector<std::string> getCollections() const {
        return collections;
    }
    
    
    friend bool operator==(const Sf1Node& left, const Sf1Node& right) {
        return (left.path == right.path and
                left.host == right.host and
                left.port == right.port and
                left.collections == right.collections);
    }
    
    
    friend bool operator!=(const Sf1Node& left, const Sf1Node& right) {
        return (left.path != right.path or
                left.host != right.host or
                left.port != right.port or
                left.collections != right.collections);
    }
    
    /**
     * Prints this instance in the format: 
     * "host:port [ collection collection ... ]"
     */
    friend std::ostream& operator<<(std::ostream& os, const Sf1Node& n) {
        os << n.host << ":" << n.port;
        os << " [ ";
        BOOST_FOREACH(std::string coll, n.collections) {
            os << coll << " ";
        }
        os << "] (" << n.path << ")";
        return os;
    }
    
private:
    
    std::string path;
    std::string host;
    uint32_t port;
    std::string service_state;
    std::vector<std::string> collections;
    
};

NS_IZENELIB_SF1R_END


#endif	/* SF1NODE_HPP */

