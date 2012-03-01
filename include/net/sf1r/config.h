/* 
 * File:   config.h
 * Author: Paolo D'Apice
 *
 * Created on February 3, 2012, 10:11 AM
 */

#ifndef CONFIG_H
#define	CONFIG_H

#include "types.h"

/** SF1 Driver namespace. */
#define NS_IZENELIB_SF1R izenelib::net::sf1r

/** SF1 Driver namespace begin. */
#define NS_IZENELIB_SF1R_BEGIN \
        namespace izenelib { namespace net { namespace sf1r {

/** SF1 Driver namespace end. */
#define NS_IZENELIB_SF1R_END \
        }}} /* namespace izenelib::net::sf1r */


#endif	/* CONFIG_H */
