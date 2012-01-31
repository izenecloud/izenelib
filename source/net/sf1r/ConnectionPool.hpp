/* 
 * File:   ConnectionPool.hpp
 * Author: Paolo D'Apice
 *
 * Created on January 31, 2012, 10:53 AM
 */

#ifndef CONNECTIONPOOL_HPP
#define	CONNECTIONPOOL_HPP

#include <boost/noncopyable.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/tuple/tuple.hpp>


namespace ba = boost::asio;


namespace izenelib {
namespace net {
namespace sf1r {


class RawClient;


/**
 * Exception thrown on connection pool errors.
 */
class ConnectionPoolError : public std::runtime_error {
public:
    ConnectionPoolError(const std::string& m = "") : std::runtime_error(m) {}
};


/**
 * Container of connection pool information.
 */
typedef boost::tuple<size_t, size_t, size_t, bool, size_t> PoolInfo;

/**
 * Enumeration for \ref PoolInfo fields.
 */
enum {
    POOL_SIZE,
    POOL_AVAILABLE,
    POOL_BUSY,
    POOL_RESIZE_FLAG,
    POOL_MAXSIZE
};


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
     * @param resize enable automatic size increment if all the clients are busy.
     * @param maxSize maximum pool size. Must hold that: maxSize >= size.
     */
    ConnectionPool(ba::io_service& service, 
                   ba::ip::tcp::resolver::iterator& iterator,
                   const size_t& size, const bool resize,
                   const size_t& maxSize);
    
    /// Destructor.
    ~ConnectionPool();
    
    /**
     * @return The actual pool information.
     * @see PoolInfo
     */
    PoolInfo getInfo() const;
    
    /**
     * Get an available client from the pool.
     * @return a reference to a \ref RawClient.
     * @throws ConnectionPoolError if there is no available client.
     */
    RawClient& acquire() throw(ConnectionPoolError);
    
    /**
     * Gives back the pool a client .
     */
    void release();
    
private:
    
    boost::mutex mutex;
    
    ba::io_service& service;
    ba::ip::tcp::resolver::iterator& iterator;
    
    size_t poolSize;
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


inline PoolInfo
ConnectionPool::getInfo() const {
    return boost::make_tuple(poolSize,
                             available.size(),
                             reserved.size(),
                             resize,
                             maxSize);
}


}}} /* namespace izenelib::net::sf1r */


#endif	/* CONNECTIONPOOL_HPP */
