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

using std::string;


const string 
ConnectionPool::UNDEFINED_PATH = "";

#define GET_PATH(path) \
       ((ConnectionPool::UNDEFINED_PATH == (path)) ? "" : " (" + (path) + ")")


ConnectionPool::ConnectionPool(ba::io_service& serv, 
                               ba::ip::tcp::resolver::iterator& it,
                               const size_t& sz, const bool rz, 
                               const size_t& ms, const string& zkpath) 
            : service(serv), iterator(it), 
              size(sz), resize(rz), maxSize(ms), path(zkpath),
              busy(0) {
    DLOG(INFO) << "Initializing pool ..." << GET_PATH(path) ;
    DLOG(INFO) << "  size       : " << size;
    DLOG(INFO) << "  resize     : " << (resize ? "true" : "false");
    DLOG(INFO) << "  maxSize    : " << maxSize;
    
    for (unsigned i = 0; i < size; ++i) {
        available.push_back(new RawClient(service, iterator, path));
    }
    
#ifdef ENABLE_SF1_TEST  
    invariant();
#endif
    
    LOG(INFO) << "Initialized pool of " << size << " clients." << GET_PATH(path) ;
}


ConnectionPool::~ConnectionPool() {
    DLOG(INFO) << "Pool released."<< GET_PATH(path) ;
}

#ifdef ENABLE_SF1_TEST
bool
ConnectionPool::invariant() const {
    CHECK(size > 0) << "invariant: poolsize == 0";
    if (resize) CHECK(maxSize >= size) << "invariant: maxSize < poolSize";
    CHECK_EQ(busy, reserved.size()) << "invariant: busy counter mismatch";
    CHECK_EQ(size, available.size() + reserved.size()) << "invariant: poolSize counter mismatch";
    return true;
}
#endif


RawClient&
ConnectionPool::acquire() throw(ConnectionPoolError) {
    DLOG(INFO) << "Connection requested."<< GET_PATH(path) ;
    boost::lock_guard<boost::mutex> lock(mutex);
    
    if (not available.empty()) {
        // move from available to reserved;
        reserved.transfer(reserved.end(), available.begin(), available);
        ++busy;
        
        return reserved.back();
    } 
    
    LOG(INFO) << "No available client."<< GET_PATH(path) ;
    
    if (not resize or size == maxSize) {
        const string msg = resize ? 
                "No available client (max size reached)" :
                "No available client (no resize)" ;
        LOG(ERROR) <<"ERROR: " << msg << GET_PATH(path) ;                   
        throw ConnectionPoolError(msg);
    }
    
    LOG(INFO) << "Growing pool ..." << GET_PATH(path) ;
    reserved.push_back(new RawClient(service, iterator, path));
    ++size;
    ++busy;
    LOG(INFO) << "Growed pool size: " << size << GET_PATH(path) ;
    
    return reserved.back();
}


void
ConnectionPool::release() {
    DLOG(INFO) << "Connection released" << GET_PATH(path) ;
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
