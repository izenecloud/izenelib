/* 
 * File:   Sf1DistributedDriver.cpp
 * Author: Paolo D'Apice
 * 
 * Created on March 1, 2012, 11:47 AM
 */

#include "net/sf1r/distributed/Sf1DistributedDriver.hpp"
#include "net/sf1r/distributed/ZooKeeperRouter.hpp"
#include "../RawClient.hpp"
#include "../Releaser.hpp"
#include <glog/logging.h>


NS_IZENELIB_SF1R_BEGIN

using boost::system::system_error;
using std::string;
using std::vector;


Sf1DistributedDriver::Sf1DistributedDriver(const string& zkhosts, 
        const Sf1Config& parameters, const Format& format)
try : Sf1DriverBase(parameters, format), hosts(zkhosts) {
    LOG(INFO) << "Driver ready.";
} catch (system_error& e) {
    string message = e.what();
    LOG(ERROR) << message;
    throw ServerError(e.what());
}


Sf1DistributedDriver::~Sf1DistributedDriver() {
    LOG(INFO) << "Driver closed.";
}


string
Sf1DistributedDriver::call(const string& uri, const string& tokens, string& request) {
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

inline void
Sf1DistributedDriver::beforeAcquire() {
    if (router.get() == NULL) {
        LOG(INFO) << "Initializing routing";
        router.reset(new ZooKeeperRouter(factory.get(), hosts, config.timeout));
    }
}

inline RawClient&
Sf1DistributedDriver::acquire(const string& collection) const {
    DLOG(INFO) << "Getting connection for collection: " << collection;
    return router->getConnection(collection);
}


inline void
Sf1DistributedDriver::release(const RawClient& client) const {
    DLOG(INFO) << "Releasing connection for: " << client.getPath();
    router->releaseConnection(client);
}


NS_IZENELIB_SF1R_END
