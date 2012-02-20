/* 
 * File:   Sf1Node.hpp
 * Author: Paolo D'Apice
 *
 * Created on February 20, 2012, 10:35 AM
 */

#ifndef SF1NODE_HPP
#define	SF1NODE_HPP

#include "config.h"
#include "types.h"
#include "util/kv2string.h"
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
     * @param nodePath
     * @param data
     */
    Sf1Node(const std::string& nodePath,
            const std::string& data);
    
    /**
     * Destructor.
     */
    ~Sf1Node();
    
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
     * @return TODO
     */
    uint32_t getBaPort() const {
        return baPort;
    }
    
    /**
     * @return TODO
     */
    uint32_t getDataPort() const {
        return dataPort;
    }
    
    /**
     * @return TODO
     */
    uint32_t getMasterPort() const {
        return masterPort;
    }
    
    /**
     * @return The collections associated to this node.
     */
    std::vector<std::string> getCollections() const {
        return collections;
    }
    
    friend std::ostream& operator<< (std::ostream& os, const Sf1Node& n) {
        os << n.host << ":" << n.baPort;
        os << " [ ";
        BOOST_FOREACH(std::string coll, n.collections) {
            os << coll << " ";
        }
        os << "]";
        return os;
    }
    
private:
    static izenelib::util::kv2string parser;
    
    std::string path;
    std::string host;
    uint32_t dataPort;
    uint32_t baPort;
    uint32_t masterPort;
    std::vector<std::string> collections;
    
};

NS_IZENELIB_SF1R_END


#endif	/* SF1NODE_HPP */

