/* 
 * File:   Sf1DistributedDriver.hpp
 * Author: Paolo D'Apice
 *
 * Created on March 1, 2012, 11:47 AM
 */

#ifndef SF1DISTRIBUTEDDRIVER_HPP
#define	SF1DISTRIBUTEDDRIVER_HPP

#include "../Sf1DriverBase.hpp"
#include "Sf1DistributedConfig.hpp"
#include "RegexLexer.hpp"
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>


NS_IZENELIB_SF1R_BEGIN

namespace ba = boost::asio;


class ZooKeeperRouter;


/**
 * Distributed SF1 driver.
 * Connects to a ZooKeeper server for SF1 topology.
 */
class Sf1DistributedDriver : public Sf1DriverBase {
public:
    
    /**
     * Creates a new instance of the driver. 
     * @param hosts list of ZooKeeper servers formatted as "host:port[,host:port]".
     * @param parameters configuration parameters.
     * @param format The format of request/response body (defaults to JSON).
     */
    Sf1DistributedDriver(const std::string& hosts, const Sf1DistributedConfig& parameters, 
              const Format& format = JSON);
    
    /// Destructor.
    ~Sf1DistributedDriver();
    
    /**
     * Sends a synchronous request to a running SF1 and get the response.
     * @see Sf1DriverBase.call()
     */
    std::string call(const std::string& uri, const std::string& tokens,
                     std::string& request);
    
private:
    
    /**
     * Dispatch a single request.
     * @param[in] uri
     * @param[in] tokens
     * @param[in] collection
     * @param[in,out] request
     * @param[out] response
     */
    void dispatchRequest(const std::string& uri, const std::string& tokens,
                         const std::string& collection, std::string& request,
                         std::string& response);
    
    /**
     * Broadcast a request.
     * @param[in] uri
     * @param[in] tokens
     * @param[in] collection
     * @param[in,out] request
     * @param[out] responses
     * @return \c true if success, \c false otherwise.
     */
    bool broadcastRequest(const std::string& uri, const std::string& tokens,
                          const std::string& collection, std::string& request, 
                          std::vector<std::string>& responses);
    
    /// Acquire a connection from the ZooKeeper router.
    RawClient& acquire(const std::string& collection) const;
    
    /// Release a connection to the ZooKeeper router.
    void release(const RawClient& connection) const;
    
private:
    
    /// ZooKeeper servers.
    const std::string hosts;
    
    /// Actual driver configuration.
    Sf1DistributedConfig config;
    
    /// The ZooKeeper router.
    boost::scoped_ptr<ZooKeeperRouter> router;
    
    /// The URI matchers
    boost::ptr_vector<RegexLexer> matchers;
    
};


NS_IZENELIB_SF1R_END


#endif	/* SF1DISTRIBUTEDDRIVER_HPP */
