/* 
 * File:   ConnectionPool.hpp
 * Author: Paolo D'Apice
 *
 * Created on January 31, 2012, 10:53 AM
 */

#ifndef CONNECTIONPOOL_HPP
#define	CONNECTIONPOOL_HPP

#include "net/sf1r/config.h"
#include "net/sf1r/Errors.hpp"
#include <boost/noncopyable.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/thread/mutex.hpp>


namespace ba = boost::asio;


NS_IZENELIB_SF1R_BEGIN


class RawClient;


/**
 * Connection pool to the SF1.
 */
class ConnectionPool : private boost::noncopyable {
public:
    
    /**
     * Initializes a connection pool.
     * @param service 
     * @param iterator
     * @param size the initial pool size. (non-zero)
     * @param resize enable automatic size increment if all the clients
     *          are busy. (default = false)
     * @param maxSize maximum pool size. It is mandatory if \ref resize 
     *          is \c true. Must hold that: maxSize >= size.
     */
    ConnectionPool(ba::io_service& service, 
                   ba::ip::tcp::resolver::iterator& iterator,
                   const size_t& size, const bool resize = false,
                   const size_t& maxSize = 0);
    
    /// Destructor.
    ~ConnectionPool();
    
    /**
     * Get an available client from the pool.
     * @return a reference to a \ref RawClient.
     * @throw ConnectionPoolError if there is no available client.
     */
    RawClient& acquire() throw(ConnectionPoolError);
    
    /**
     * Gives back the pool a client .
     */
    void release();
    
    /**
     * @return The actual pool size (number of connections).
     */
    size_t getSize() const {
        return size;
    }
    
    /**
     * @return The number of available connections.
     */
    size_t getAvailableSize() const {
        return available.size();
    }
    
    /**
     * @return The number of connections currently busy.
     */
    size_t getReservedSize() const {
        return reserved.size();
    }
    
    /**
     * @return \c true if the pool can automatically increase its size.
     */
    bool isResizable() const {
        return resize;
    }
    
    /**
     * @return The maximum number of connections that can be used by the pool.
     */
    size_t getMaxSize() const {
        return maxSize;
    }
    
private:
    
    boost::mutex mutex;
    
    ba::io_service& service;
    ba::ip::tcp::resolver::iterator& iterator;
    
    size_t size;
    const bool resize;
    const size_t maxSize;
    
    // Aliases for the actual container used.
    typedef boost::ptr_list<RawClient> Container;
    typedef Container::iterator Iterator;
    
    Container available;
    Container reserved;
    size_t busy;
    
public:
    /// Check the representation invariant (for tests only).
    bool invariant() const;
};


NS_IZENELIB_SF1R_END


#endif	/* CONNECTIONPOOL_HPP */
