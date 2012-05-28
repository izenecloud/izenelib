/* 
 * File:   common.hpp
 * Author: Paolo D'Apice
 *
 * Created on May 25, 2012, 11:08 AM
 */

#ifndef IZENELIB_NET_DISTRIBUTE_COMMON_HPP
#define	IZENELIB_NET_DISTRIBUTE_COMMON_HPP

#include <cstddef>


#define NS_IZENELIB_DISTRIBUTE izenelib::net::distribute

#define NS_IZENELIB_DISTRIBUTE_BEGIN \
        namespace izenelib { namespace net { namespace distribute {

#define NS_IZENELIB_DISTRIBUTE_END \
        }}} /* izenelib::net::distribute */


namespace {
/// Default buffer size. Value: 64 KiB.
const size_t DEFAULT_BUFFER_SIZE = 64 * 1024;
/// Default port. Value: 18121.
const unsigned DEFAULT_PORT = 18121;
}


#endif	/* IZENELIB_NET_DISTRIBUTE_COMMON_HPP */
