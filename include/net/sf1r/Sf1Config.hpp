/* 
 * File:   Sf1Config.hpp
 * Author: paolo
 *
 * Created on March 1, 2012, 9:35 AM
 */

#ifndef SF1CONFIG_HPP
#define	SF1CONFIG_HPP


/**
 * Container for the driver configuration parameters:
 * - initial pool size
 * - automatic pool resize
 * - maximum pool size
 */
struct Sf1Config {
    Sf1Config(const size_t& s, const bool r, const size_t& ms = 0)
            : initialSize(s), resize(r), maxSize(ms) {}
    
    const size_t initialSize;           ///< Initial pool size.
    const bool resize;                  ///< Automatic pool resize.
    const size_t maxSize;               ///< Maximum pool size.
};


#endif	/* SF1CONFIG_HPP */
