/* 
 * File:   PoolFactory.hpp
 * Author: Paolo D'Apice
 *
 * Created on March 1, 2012, 2:40 PM
 */

#ifndef POOLFACTORY_HPP
#define	POOLFACTORY_HPP

#include "net/sf1r/config.h"
#include "net/sf1r/Sf1Config.hpp"
#include "net/sf1r/distributed/Sf1Node.hpp"
#include "ConnectionPool.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <glog/logging.h>
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
     * @param service The I/O service.
     * @param config The actual configuration structure.
     */
    PoolFactory(ba::io_service& _service, const Sf1Config& _config)
            : service(_service), config(_config) {
        DLOG(INFO) << "PoolFactory ready";
    }
    
    /// Desctructor.
    ~PoolFactory() {
        DLOG(INFO) << "PoolFactory destroyed";
    }
    
    /**
     * Instantiates a new connection pool.
     * @param The target address.
     * @return A pointer to a new ConnectionPool.
     * @throw NetworkError if network-related errors occur
     */
    ConnectionPool* newConnectionPool(const std::string& address) const {
        LOG(INFO) << "new connection pool to: [" << address << "]";
        
        size_t pos = address.find(':');
        return new ConnectionPool(service, 
                address.substr(0, pos), address.substr(pos + 1),
                config.timeout,
                config.initialSize, config.resize, config.maxSize);
    }
    
    /**
     * Instantiates a new connection pool.
     * @param The target SF1 node.
     * @return A pointer to a new ConnectionPool.
     * @throw NetworkError if network-related errors occur
     */
    ConnectionPool* newConnectionPool(const Sf1Node& node) const {
        LOG(INFO) << "new connection pool to: [" << node << "]";
        
        return new ConnectionPool(service, node.getHost(), 
                boost::lexical_cast<std::string>(node.getPort()),
                config.timeout,
                config.initialSize, config.resize, config.maxSize,
                node.getPath());
    }
    
private:
    ba::io_service& service;
    const Sf1Config config;
};


NS_IZENELIB_SF1R_END


#endif	/* POOLFACTORY_HPP */

