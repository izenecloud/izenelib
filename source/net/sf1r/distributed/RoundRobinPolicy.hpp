/* 
 * File:   RoundRobinPolicy.hpp
 * Author: Paolo D'Apice
 *
 * Created on March 5, 2012, 2:59 PM
 */

#ifndef ROUNDROBINPOLICY_HPP
#define	ROUNDROBINPOLICY_HPP

#include "RoutingPolicy.hpp"
#include "net/sf1r/distributed/NodeContainer.hpp"
#include <boost/ptr_container/ptr_map.hpp>
#include <map>

NS_IZENELIB_SF1R_BEGIN

/**
 * Round-robin routing policy.
 * The policy is implemented simulating a circular buffer 
 * using a modulo counter for direct access.
 */
class RoundRobinPolicy : public RoutingPolicy {
public:
    
    /// Constructor.
    RoundRobinPolicy(Sf1Topology&, Sf1Topology& backup_nodes);
    
    /// Destructor.
    ~RoundRobinPolicy();
    
    /// Get a node using a round-robin policy.
    const Sf1Node& getNode();
    
    /// Get a node hosting the given collection using a round-robin policy.
    const Sf1Node& getNodeFor(const std::string& collection);

    void increSlowCounter(const std::string& nodepath);
    void updateSlowCounterForAll();

private:  
    
    /// Reset the index counter.
    void resetCounter();
    
    /// Update the map of collections.
    void updateCollections();
    const Sf1Node& getNodeFor(const std::string& collection, const NodeCollectionsList& chose_from);
    const Sf1Node& getBackupNodeFor(const std::string& collection);
    
private:
    
    /// Counter used for randomly access all nodes in topology.
    size_t counter;
    
    /// Map collections to node lists.
    boost::ptr_map<std::string, NodeCollectionsList> collections;
    /// Map of counters used for randomly access nodes hosting a collection.
    std::map<std::string, size_t> ccounter;
    std::map<std::string, size_t> slow_counter;
    boost::ptr_map<std::string, NodeCollectionsList> backup_collections;
    Sf1Topology& backup_topology;
    
};

NS_IZENELIB_SF1R_END

#endif	/* ROUNDROBINPOLICY_HPP */
