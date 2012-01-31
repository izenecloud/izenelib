/* 
 * File:   ConnectionPool.cpp
 * Author: Paolo D'Apice
 * 
 * Created on January 31, 2012, 10:53 AM
 */

#include "ConnectionPool.hpp"
#include "RawClient.hpp"
#include <boost/assert.hpp>
#include <boost/thread.hpp>
#include <boost/tuple/tuple_io.hpp>
#include <glog/logging.h>

namespace izenelib {
namespace net {
namespace sf1r {


ConnectionPool::ConnectionPool(ba::io_service& serv, 
                               ba::ip::tcp::resolver::iterator& it,
                               const size_t& size, const bool rz, 
                               const size_t& ms) 
            : service(serv), iterator(it), 
              poolSize(size), resize(rz), maxSize(ms),
              busy(0) {
    DLOG(INFO) << "Initializing pool ...";
    DLOG(INFO) << " poolSize   : " << poolSize;
    DLOG(INFO) << " resize     : " << (resize ? "true" : "false");
    DLOG(INFO) << " maxSize    : " << maxSize;
    
    for (unsigned i = 0; i < size; ++i) {
        available.push_back(new RawClient(service, iterator));
    }
    
    LOG(INFO) << "Initialized pool of " << size << " clients.";
}


ConnectionPool::~ConnectionPool() {
    DLOG(INFO) << "Pool released.";
}


bool
ConnectionPool::invariant() const {
    CHECK(poolSize > 0) << "poolsize == 0";
    CHECK(maxSize >= poolSize) << "maxSize < poolSize";
    CHECK_EQ(busy, reserved.size()) << "busy counter mismatch";
    CHECK_EQ(poolSize, available.size() + reserved.size()) << "poolSize counter mismatch";
    return true;
}


RawClient&
ConnectionPool::acquire() throw(ConnectionPoolError) {
    DLOG(INFO) << "connection requested";
    boost::lock_guard<boost::mutex> lock(mutex);
    
    if (not available.empty()) {
        // move from available to reserved;
        reserved.transfer(reserved.end(), available.begin(), available);
        ++busy;
        
        return reserved.back();
    } 
    
    LOG(INFO) << "No available client";
    
    if (not resize or poolSize == maxSize) {
        const std::string msg = resize ? 
                "No available client (max size reached)" :
                "No available client (no resize)" ;
        LOG(ERROR) << "ERROR: " << msg;                   
        throw ConnectionPoolError(msg);
    }
    
    LOG(INFO) << "Grow pool";
    reserved.push_back(new RawClient(service, iterator));
    ++poolSize;
    ++busy;
    
    return reserved.back();
}


void
ConnectionPool::release() {
    DLOG(INFO) << "connection released";
    boost::lock_guard<boost::mutex> lock(mutex); 
    
    for (Iterator it = reserved.begin(); it != reserved.end(); ++it) {
        if (it->idle()) {
            // move from available to reserved;
            available.transfer(available.end(), it, reserved);
            --busy;
            
            break;
        }
    }
}


}}} /* namespace izenelib::net::sf1r */
