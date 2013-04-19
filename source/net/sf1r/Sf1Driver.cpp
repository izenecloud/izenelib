/* 
 * File:   Sf1Driver.cpp
 * Author: Paolo D'Apice
 * 
 * Created on January 10, 2012, 10:07 AM
 */

#include "net/sf1r/Sf1Driver.hpp"
#include "ConnectionPool.hpp"
#include "PoolFactory.hpp"
#include "Releaser.hpp"
#include <glog/logging.h>


NS_IZENELIB_SF1R_BEGIN


using ba::ip::tcp;
using std::string;
using std::vector;


Sf1Driver::Sf1Driver(const string& h, const Sf1Config& parameters, 
        const Format& fmt) : Sf1DriverBase(parameters, fmt), host(h), 
        mustReconnect(true) {
    LOG(INFO) << "Driver ready.";
}


Sf1Driver::~Sf1Driver() {
    LOG(INFO) << "Driver closed.";
}


std::string
Sf1Driver::call(const string& uri, const string& tokens, string& request) {
    bool retry = false;
    do {
        // lazy initialization
        if (mustReconnect or pool.get() == NULL) {
            reconnect();
        }
        
        string controller, action;
        parseUri(uri, controller, action);
        
        string collection;
        preprocessRequest(controller, action, tokens, request, collection);
        
        LOG(INFO) << "Send " << getFormatString() << " request: " << request;
        
        incrementSequence();
        
        RawClient& client = acquire(collection);
        
        // process request
        Releaser r(*this, client);
        try {
            string response;
            sendAndReceive(client, request, response); 
            return response;
        } catch (NetworkError& e) {
            LOG(WARNING) << e.what();
            mustReconnect = true;
            DLOG(INFO) << "Reconnect flag set to: " << mustReconnect;
            if (not retry) { // retry only once
                retry = true;
                LOG(INFO) << "Retry flag set to: " << retry;
            } else {
                throw e;
            }
        } catch (ServerError& e) { // do not intercept ServerError
            DLOG(INFO) << e.what();
            throw e;
        }
    } while (retry);
    throw ServerError("");
    return "";
}


inline RawClient&
Sf1Driver::acquire(const string&) const {
    DLOG(INFO) << "Acquiring connection ...";
    return pool->acquire();
}


inline void
Sf1Driver::release(const RawClient& connection) const {
    DLOG(INFO) << "Releasing connection ...";
    pool->release(connection);
}


void
Sf1Driver::reconnect() {
    CHECK(mustReconnect) << "Should not reconnect";
        
    try {
        LOG(INFO) << "Connection pool may be invalid, replacing ...";
        pool.reset(factory->newConnectionPool(host));
        mustReconnect = false;
    } catch (NetworkError& e) {
        LOG(ERROR) << e.what();
        throw e;
    }
}


inline size_t 
Sf1Driver::getPoolSize() const {
    return pool->getSize();
}


NS_IZENELIB_SF1R_END
