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


NS_IZENELIB_SF1R_BEGIN


ConnectionPool::ConnectionPool(ba::io_service& serv, 
                               ba::ip::tcp::resolver::iterator& it,
                               const size_t& sz, const bool rz, 
                               const size_t& ms) 
            : service(serv), iterator(it), 
              size(sz), resize(rz), maxSize(ms),
              busy(0) {
    DLOG(INFO) << "Initializing pool ...";
    DLOG(INFO) << " poolSize   : " << size;
    DLOG(INFO) << " resize     : " << (resize ? "true" : "false");
    DLOG(INFO) << " maxSize    : " << maxSize;
    
    for (unsigned i = 0; i < size; ++i) {
        available.push_back(new RawClient(service, iterator));
    }
    
    invariant();
    LOG(INFO) << "Initialized pool of " << size << " clients.";
}


ConnectionPool::~ConnectionPool() {
    DLOG(INFO) << "Pool released.";
}


bool
ConnectionPool::invariant() const {
    CHECK(size > 0) << "invariant: poolsize == 0";
    if (resize) CHECK(maxSize >= size) << "invariant: maxSize < poolSize";
    CHECK_EQ(busy, reserved.size()) << "invariant: busy counter mismatch";
    CHECK_EQ(size, available.size() + reserved.size()) << "invariant: poolSize counter mismatch";
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
    
    if (not resize or size == maxSize) {
        const std::string msg = resize ? 
                "No available client (max size reached)" :
                "No available client (no resize)" ;
        LOG(ERROR) << "ERROR: " << msg;                   
        throw ConnectionPoolError(msg);
    }
    
    LOG(INFO) << "Grow pool";
    reserved.push_back(new RawClient(service, iterator));
    ++size;
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


NS_IZENELIB_SF1R_END
