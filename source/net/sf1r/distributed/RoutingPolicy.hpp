/* 
 * File:   RoutingPolicy.hpp
 * Author: Paolo D'Apice
 *
 * Created on March 5, 2012, 2:47 PM
 */

#ifndef ROUTINGPOLICY_HPP
#define	ROUTINGPOLICY_HPP

#include "net/sf1r/config.h"
#include <boost/noncopyable.hpp>
#include <string>

NS_IZENELIB_SF1R_BEGIN

class Sf1Node;
class Sf1Topology;

/**
 * Abstract class for routing policies.
 */
class RoutingPolicy : boost::noncopyable {
public:
    
    /**
     * Constructor.
     * @param topology_ The actual topology.
     */
    RoutingPolicy(Sf1Topology& topology_) : topology(topology_) {}
    
    /// Destructor.
    virtual ~RoutingPolicy() {}
    
    /**
     * Get a node from the actual topology.
     * @return A const reference to a node.
     */
    virtual const Sf1Node& getNode() = 0;
    
    /**
     * Get a node hosting the given collection from the actual topology.
     * @param collection The collection name
     * @return A const reference to a node.
     */
    virtual const Sf1Node& getNodeFor(const std::string& collection) = 0;
    
    virtual void increSlowCounter(const std::string& nodepath) = 0;
    virtual void updateSlowCounterForAll() = 0;

protected:
    
    Sf1Topology& topology;
};

NS_IZENELIB_SF1R_END


#endif	/* ROUTINGPOLICY_HPP */
