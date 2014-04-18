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

using std::string;
using std::vector;


Sf1DistributedDriver::Sf1DistributedDriver(const string& zkhosts, 
        const Sf1DistributedConfig& parameters, const Format& format)
      : Sf1DriverBase(parameters, format), hosts(zkhosts), config(parameters) {
    DLOG(INFO) << "Initializing matchers ...";
    BOOST_FOREACH(const string& pattern, config.broadcast) {
        matchers.push_back(new RegexLexer(pattern));
        DLOG(INFO) << "Added broadcast matcher for [" << pattern << "]";
    }
    try {
        LOG(INFO) << "Initializing routing";
        router.reset(new ZooKeeperRouter(factory.get(), hosts, config.zkTimeout,
                config.match_master_name, config.set_seq, config.total_set_num));
    } catch (izenelib::zookeeper::ZooKeeperException& e) {
        LOG(ERROR) << "Initializing routing failed in: " << (long)getpid();
        router.reset();
        LOG(ERROR) << e.what();
        throw NetworkError(e.what());
    }

    LOG(INFO) << "Driver ready.";
}


Sf1DistributedDriver::~Sf1DistributedDriver() {
    LOG(INFO) << "Driver closed.";
}


string
Sf1DistributedDriver::call(const string& uri, const string& tokens, string& request) {
    // lazy initialization
    if (router.get() == NULL) {
        try {
            LOG(INFO) << "Initializing routing";
            router.reset(new ZooKeeperRouter(factory.get(), hosts, config.zkTimeout,
                    config.match_master_name, config.set_seq, config.total_set_num));
        } catch (izenelib::zookeeper::ZooKeeperException& e) {
            LOG(ERROR) << e.what();
            router.reset();
            throw NetworkError(e.what());
        }
    }
    
    string controller, action;
    parseUri(uri, controller, action);
    
    string collection;
    preprocessRequest(controller, action, tokens, request, collection);
    
    string response;
    
    /*
     * Matchers are iterated on reverse order, so that the last rule
     * has the precedence. Only one match is allowed.
     */
    BOOST_REVERSE_FOREACH(const RegexLexer& m, matchers) {
        DLOG(INFO) << "Matching [" << uri << "] against: [" << m.regex() << "]";
            
        if (m.match(uri)) {
            DLOG(INFO) << "Matched, broadcasting ...";
            std::vector<string> responses;
            bool success = broadcastRequest(uri, tokens, collection, request, responses);
            if (not success) { // FIXME: really throw?
                LOG(WARNING) << "Not all requests succeeded";
                throw ServerError("Unsuccessful broadcast");
            }
            
            DLOG(INFO) << "Returning the last response";
            BOOST_ASSERT(not responses.empty());
            response.assign(responses.back());
            return response;
        }
    }
    
    DLOG(INFO) << "No matches, default dispatching ...";
    dispatchRequest(uri, tokens, collection, request, response);
    
    return response;
}


void 
Sf1DistributedDriver::dispatchRequest(const string& uri, const string& tokens, 
        const string& collection, string& request, string& response) {
    //LOG(INFO) << "Send " << getFormatString() << " request: " << request;

    incrementSequence();

    RawClient& client = acquire(collection);

    // process request
    Releaser r(*this, client);
    try {
        time_t start_time = time(NULL);
        sendAndReceive(client, request, response); 
        time_t end_time = time(NULL);
        if (end_time - start_time > 7)
        {
            LOG(INFO) << "slow request from server : " << client.getPath() << ", cost time: " << end_time-start_time << ", " << request;
            if (request.find("\"keywords\":\"*\"") != std::string::npos)
            {
                LOG(INFO) << "The * search can be slow, skip.";
            }
            else
            {
                router->increSlowCounter(client.getPath());
            }
        }
    } catch (const NetworkError& e) { // do not intercept ServerError
        LOG(INFO) << "NetworkError request from server : " << client.getPath() << ", " << request;
        router->increSlowCounter(client.getPath());
        throw;
    } catch (const std::runtime_error& e) {
        LOG(ERROR) << "Exception: " << e.what();
        throw;
    }
}


bool 
Sf1DistributedDriver::broadcastRequest(const string& uri, const string& tokens,
        const string& collection, string& request, vector<std::string>& responses) {
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
