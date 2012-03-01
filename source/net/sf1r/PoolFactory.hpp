/* 
 * File:   PoolFactory.hpp
 * Author: paolo
 *
 * Created on March 1, 2012, 2:40 PM
 */

#ifndef POOLFACTORY_HPP
#define	POOLFACTORY_HPP

#include "net/sf1r/config.h"
#include "net/sf1r/Sf1Config.hpp"
#include "ConnectionPool.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <string>

NS_IZENELIB_SF1R_BEGIN

namespace ba = boost::asio;


/**
 * ConnectioPool factory.
 */
class PoolFactory : boost::noncopyable {
public:
    
    /**
     * Instantiates a new factory. 
     */
    PoolFactory(ba::io_service& _service, ba::ip::tcp::resolver& _resolver, Sf1Config& _config)
        : service(_service), resolver(_resolver), config(_config) {}
    
    /**
     * Instantiates a new connection pool.
     */
    ConnectionPool* newConnectionPool(const std::string& host, const uint32_t& port) {
        ba::ip::tcp::resolver::query query(host, boost::lexical_cast<std::string>(port));
        ba::ip::tcp::resolver::iterator iterator = resolver.resolve(query);
    
        return new ConnectionPool(service, iterator, 
                config.initialSize, config.resize, config.maxSize);
    }
    
private:
    ba::io_service& service;
    ba::ip::tcp::resolver& resolver;
    Sf1Config& config;
};


NS_IZENELIB_SF1R_END


#endif	/* POOLFACTORY_HPP */

