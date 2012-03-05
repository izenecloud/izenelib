/* 
 * File:   Sf1Driver.cpp
 * Author: paolo
 * 
 * Created on January 10, 2012, 10:07 AM
 */

#include "net/sf1r/Sf1Driver.hpp"
#include "ConnectionPool.hpp"
#include "PoolFactory.hpp"
#include <glog/logging.h>


NS_IZENELIB_SF1R_BEGIN


using ba::ip::tcp;
using boost::system::system_error;
using std::string;
using std::vector;


Sf1Driver::Sf1Driver(const string& host, const uint32_t& port, 
        const Sf1Config& parameters, const Format& fmt) throw(ServerError) 
    try : Sf1DriverBase(parameters, fmt), pool(factory->newConnectionPool(host, port)) {
    LOG(INFO) << "Driver ready.";
} catch (system_error& e) {
    string message = e.what();
    LOG(ERROR) << message;
    throw ServerError(message);
}


Sf1Driver::~Sf1Driver() {
    LOG(INFO) << "Driver closed.";
}


RawClient&
Sf1Driver::acquire(const std::string&) const {
    DLOG(INFO) << "Acquiring connection ...";
    return pool->acquire();
}


void
Sf1Driver::release(const RawClient&) const {
    DLOG(INFO) << "Releasing connection ...";
    pool->release();
}


inline size_t 
Sf1Driver::getPoolSize() const {
    return pool->getSize();
}


NS_IZENELIB_SF1R_END
