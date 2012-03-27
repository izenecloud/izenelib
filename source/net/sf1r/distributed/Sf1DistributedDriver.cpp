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
#include <boost/foreach.hpp>
#include <glog/logging.h>


NS_IZENELIB_SF1R_BEGIN

using boost::system::system_error;
using std::string;
using std::vector;


Sf1DistributedDriver::Sf1DistributedDriver(const string& zkhosts, 
        const Sf1DistributedConfig& parameters, const Format& format)
try : Sf1DriverBase(parameters, format), hosts(zkhosts), config(parameters) {
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
    
    /*
     * TODO: apply routing policy!
     * Parse URI and apply the configured policy:
     * - RoundRobin (default)
     * - Broadcast: 1 input request => N actual requests
     */
    
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


bool 
Sf1DistributedDriver::broadcastRequest(const string& uri, const string& tokens,
        const string& collection, string& request, vector<std::string>& responses) {
    // check that zookeeper has been initialized
    initZooKeeperRouter();
    
    // get all the connections
    vector<RawClient*> connections = router->getConnections(collection);
    DLOG(INFO) << "Broadcasting request to (" << connections.size() << ") nodes ...";
    bool success = true;
    BOOST_FOREACH(RawClient* connection, connections) {
        DLOG(INFO) << "Sending to " << connection->getPath() << "...";
        LOG(INFO) << "Send " << getFormatString() << " request: " << request;
                
        incrementSequence();
        
        // process request
        Releaser r(*this, *connection);
        try {
            string response;
            sendAndReceive(*connection, request, response);
            responses.push_back(response);
        } catch (std::runtime_error& e) {
            LOG(ERROR) << "Exception: " << e.what();
            success = false;
            break;
        }
    }
    
    DLOG(INFO) << "Finished broadcasting request, success = " << (success ? "yes" : "no");
    return success;
}


inline void
Sf1DistributedDriver::initZooKeeperRouter() {
    if (router.get() == NULL) {
        LOG(INFO) << "Initializing routing";
        router.reset(new ZooKeeperRouter(factory.get(), hosts, config.timeout));
    }
}


inline void
Sf1DistributedDriver::beforeAcquire() {
    // lazy initialization because of problems with Nginx
    initZooKeeperRouter();
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
