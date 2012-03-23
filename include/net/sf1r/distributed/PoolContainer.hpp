/* 
 * File:   PoolContainer.hpp
 * Author: Paolo D'Apice
 *
 * Created on March 1, 2012, 1:30 PM
 */

#ifndef POOLCONTAINER_HPP
#define	POOLCONTAINER_HPP

#include "../config.h"
#include <map>

NS_IZENELIB_SF1R_BEGIN

class ConnectionPool;


/** Cointainer of pointers to ConectionPool. */
typedef std::map<std::string, ConnectionPool*> PoolContainer;


NS_IZENELIB_SF1R_END


#endif	/* POOLCONTAINER_HPP */
