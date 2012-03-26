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
using boost::system::system_error;
using std::string;
using std::vector;


Sf1Driver::Sf1Driver(const string& host, const Sf1Config& parameters, 
        const Format& fmt) 
try : Sf1DriverBase(parameters, fmt), pool(factory->newConnectionPool(host)) {
    LOG(INFO) << "Driver ready.";
} catch (system_error& e) {
    string message = e.what();
    LOG(ERROR) << message;
    throw ServerError(message);
}


Sf1Driver::~Sf1Driver() {
    LOG(INFO) << "Driver closed.";
}


std::string
Sf1Driver::call(const string& uri, const string& tokens, string& request) {
    string controller, action;
    parseUri(uri, controller, action);
    
    string collection;
    preprocessRequest(controller, action, tokens, request, collection);
    
    LOG(INFO) << "Send " << getFormatString() << " request: " << request;
    
    incrementSequence();
    
    RawClient& client = getConnection(collection);
    
    // process request
    Releaser r(*this, client);
    try {
        string response;
        sendAndReceive(client, request, response); 
        return response;
    } catch (ServerError& e) { // do not intercept ServerError
        throw e;
    } catch (std::runtime_error& e) {
        LOG(ERROR) << "Exception: " << e.what();
        throw e;
    }
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


inline size_t 
Sf1Driver::getPoolSize() const {
    return pool->getSize();
}


NS_IZENELIB_SF1R_END
