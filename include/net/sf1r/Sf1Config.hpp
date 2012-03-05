/* 
 * File:   Sf1Config.hpp
 * Author: paolo
 *
 * Created on March 1, 2012, 9:35 AM
 */

#ifndef SF1CONFIG_HPP
#define	SF1CONFIG_HPP


#define SF1_CONFIG_INITIAL_SIZE  1
#define SF1_CONFIG_RESIZE        false
#define SF1_CONFIG_MAX_SIZE      1
#define SF1_CONFIG_TIMEOUT       2000

/**
 * Container for the driver configuration parameters:
 * - initial pool size
 * - automatic pool resize
 * - maximum pool size
 */
struct Sf1Config {
    
    /**
     * Default constructor.
     */
    Sf1Config() : initialSize(SF1_CONFIG_INITIAL_SIZE), 
        resize(SF1_CONFIG_RESIZE), maxSize(SF1_CONFIG_MAX_SIZE), 
        timeout(SF1_CONFIG_TIMEOUT) {}
    
    const size_t initialSize;           ///< Initial pool size.
    const bool resize;                  ///< Automatic pool resize.
    const size_t maxSize;               ///< Maximum pool size.
    const uint32_t timeout;             ///< ZooKeeper session timeout.
};


#endif	/* SF1CONFIG_HPP */
