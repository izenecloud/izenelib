/* 
 * File:   Sf1DistributedConfig.hpp
 * Author: Paolo D'Apice
 *
 * Created on March 26, 2012, 3:51 PM
 */

#ifndef SF1DISTRIBUTEDCONFIG_HPP
#define	SF1DISTRIBUTEDCONFIG_HPP

#include "../config.h"
#include "../Sf1Config.hpp"

NS_IZENELIB_SF1R_BEGIN


/// Default ZooKeeper session timeout.
#define SF1_CONFIG_TIMEOUT       2000

/**
 * Container for the distributed driver configuration parameters:
 * - ZooKeeper session timeout
 * - routing policies
 */
struct Sf1DistributedConfig : public Sf1Config {
    
    /**
     * Default constructor.
     */
    Sf1DistributedConfig() : Sf1Config(), 
        timeout(SF1_CONFIG_TIMEOUT) {
    }
    
    uint32_t timeout;             ///< ZooKeeper session timeout.
    
};


NS_IZENELIB_SF1R_END

#endif	/* SF1DISTRIBUTEDCONFIG_HPP */
