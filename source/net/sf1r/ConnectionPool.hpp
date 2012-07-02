/* 
 * File:   ConnectionPool.hpp
 * Author: Paolo D'Apice
 *
 * Created on January 31, 2012, 10:53 AM
 */

#ifndef CONNECTIONPOOL_HPP
#define	CONNECTIONPOOL_HPP

#include "net/sf1r/config.h"
#include <boost/noncopyable.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>


NS_IZENELIB_SF1R_BEGIN

namespace ba = boost::asio;

class RawClient;


/**
 * Connection pool to the SF1.
 */
class ConnectionPool : private boost::noncopyable {
public:
    
    /// Default undefined path.
    static const std::string UNDEFINED_PATH;
    
    /**
     * Initializes a connection pool.
     * @param service A reference to the I/O service.
     * @param host The target address or hostname.
     * @param port The target service port.
     * @param timeout The socket timeout.
     * @param size The initial pool size. (non-zero)
     * @param resize Enable automatic size increment if all the clients
     *          are busy. (defaults to false)
     * @param maxSize The maximum pool size. It is mandatory if \ref resize 
     *          is \c true. Must hold that: maxSize >= size.
     * @param path The ZooKeeper path to which the connection pool refers to.
     *          (defaults to UNDEFINED_PATH)
     * @throw NetworkError if network-related errors occur.
     */
    ConnectionPool(ba::io_service& service, 
                   const std::string& host, const std::string& port, 
                   const size_t timeout, const size_t& size, 
                   const bool resize = false, const size_t& maxSize = 0,
                   const std::string& path = UNDEFINED_PATH);
    
    /// Destructor.
    ~ConnectionPool();
    
    /**
     * Get an available connection from the pool.
     * @return a reference to a \ref RawClient.
     * @throw ConnectionPoolError if there is no available connection.
     * @throw NetworkError if network-related errors occur.
     */
    RawClient& acquire();
    
    /**
     * Gives back the pool a connection.
     * @param connection A connection
     */
    void release(const RawClient& connection);
    
    /**
     * @return The actual pool size (number of connections).
     */
    size_t getSize() {
        boost::lock_guard<boost::mutex> lock(mutex);
        return size;
    }
    
    /**
     * @return The number of available connections.
     */
    size_t getAvailableSize() {
        boost::lock_guard<boost::mutex> lock(mutex);
        return available.size();
    }
    
    /**
     * @return The number of connections currently busy.
     */
    size_t getReservedSize() {
        boost::lock_guard<boost::mutex> lock(mutex);
        return reserved.size();
    }
    
    /**
     * @return \c true if there is any connection in use.
     */
    bool isBusy() {
        boost::lock_guard<boost::mutex> lock(mutex);
        return not reserved.empty();
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
    
    /**
     * @return The ZooKeeper path.
     */
    std::string getPath() const {
        return path;
    }
    
private:
    
    /// Mutex for read/write operation on the internal connection queues.
    boost::mutex mutex;
    /// Locking condition for pool finalization.
    boost::condition_variable condition;
    
    /// Input/Output service.
    ba::io_service& service;
    
    /// The target hostname or address.
    const std::string host;
    /// The target service port.
    const std::string port;
    /// The socket timeout.
    const size_t timeout;
    
    /// The actual size.
    size_t size;
    /// Automatic resize flag.
    const bool resize;
    /// The maximum number of connections.
    const size_t maxSize;
    /// The ZooKeeper path (if defined)
    std::string path;
    
    // Aliases for the actual container used.
    typedef boost::ptr_list<RawClient> Container;
    typedef Container::iterator Iterator;
    
    /// Available connections.
    Container available;
    /// Busy connections.
    Container reserved;
    
    
#ifdef ENABLE_SF1_TEST
public:
    /// Check the representation invariant (for tests only).
    bool invariant() const;
#endif
    
};


NS_IZENELIB_SF1R_END


#endif	/* CONNECTIONPOOL_HPP */
