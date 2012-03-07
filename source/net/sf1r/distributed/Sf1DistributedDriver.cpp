/* 
 * File:   Sf1DistributedDriver.cpp
 * Author: paolo
 * 
 * Created on March 1, 2012, 11:47 AM
 */

#include "net/sf1r/distributed/Sf1DistributedDriver.hpp"
#include "net/sf1r/distributed/ZooKeeperRouter.hpp"
#include "../RawClient.hpp"
#include <glog/logging.h>


NS_IZENELIB_SF1R_BEGIN

using std::string;
using std::vector;


Sf1DistributedDriver::Sf1DistributedDriver(const string& hosts, 
        const Sf1Config& parameters, const Format& format) throw(ServerError)
try : Sf1DriverBase(parameters, format) {
    router.reset(new ZooKeeperRouter(factory.get(), hosts, config.timeout));
    LOG(INFO) << "Driver ready.";
} catch (izenelib::zookeeper::ZooKeeperException& e) {
    string message = e.what();
    LOG(ERROR) << message;
    throw ServerError(e.what());
}


Sf1DistributedDriver::~Sf1DistributedDriver() {
    LOG(INFO) << "Driver closed.";
}


RawClient&
Sf1DistributedDriver::acquire(const std::string& collection) const {
    DLOG(INFO) << "Getting connection for collection: " << collection;
    return router->getConnection(collection);
}


void
Sf1DistributedDriver::release(const RawClient& client) const {
    DLOG(INFO) << "Releasing connection for: " << client.getPath();
    router->releaseConnection(client);
}


NS_IZENELIB_SF1R_END
