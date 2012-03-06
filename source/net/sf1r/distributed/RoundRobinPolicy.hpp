/* 
 * File:   RoundRobinPolicy.hpp
 * Author: paolo
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

class RoundRobinPolicy : public RoutingPolicy {
public:
    RoundRobinPolicy(Sf1Topology&);
    ~RoundRobinPolicy();
    
    const Sf1Node& getNode();
    const Sf1Node& getNodeFor(const std::string collection);

private:  
    void resetCounter();
    void updateCollections();
    
private:
    size_t counter;
    boost::ptr_map<std::string, NodeCollectionsList> collections;
    std::map<std::string, size_t> ccounter;
    
};

NS_IZENELIB_SF1R_END

#endif	/* ROUNDROBINPOLICY_HPP */
