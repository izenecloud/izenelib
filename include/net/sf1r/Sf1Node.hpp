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
     * @param host
     * @param baPort
     * @param dataPort
     * @param masterPort
     * @param collections
     */
    Sf1Node(const std::string& host,
            const uint32_t& baPort,
            const uint32_t& dataPort,
            const uint32_t& masterPort,
            const std::string& collections);
    
    /**
     * Destructor.
     */
    ~Sf1Node();
    
    std::string getHost() const {
        return host;
    }
    
    uint32_t getBaPort() const {
        return baPort;
    }
    
    uint32_t getDataPort() const {
        return dataPort;
    }
    
    uint32_t getMasterPort() const {
        return masterPort;
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
    std::string host;
    uint32_t dataPort;
    uint32_t baPort;
    uint32_t masterPort;
    std::vector<std::string> collections;
    
};

NS_IZENELIB_SF1R_END


#endif	/* SF1NODE_HPP */

