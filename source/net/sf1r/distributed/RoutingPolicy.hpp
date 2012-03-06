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


class RoutingPolicy : boost::noncopyable {
public:
    
    RoutingPolicy(Sf1Topology& topology_) : topology(topology_) {}
    virtual ~RoutingPolicy() {}
    
    virtual const Sf1Node& getNode() = 0;
    virtual const Sf1Node& getNodeFor(const std::string collection) = 0;
    
protected:
    Sf1Topology& topology;
};

NS_IZENELIB_SF1R_END


#endif	/* ROUTINGPOLICY_HPP */
