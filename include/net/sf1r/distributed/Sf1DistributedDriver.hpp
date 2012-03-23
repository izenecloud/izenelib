/* 
 * File:   Sf1DistributedDriver.hpp
 * Author: Paolo D'Apice
 *
 * Created on March 1, 2012, 11:47 AM
 */

#ifndef SF1DISTRIBUTEDDRIVER_HPP
#define	SF1DISTRIBUTEDDRIVER_HPP

#include "../Sf1DriverBase.hpp"
#include <boost/scoped_ptr.hpp>
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
     * Creates a new instance of the driver and connects to the ZooKeper server. 
     * @param hosts list of ZooKeeper servers formatted as "host:port[,host:port]".
     * @param parameters configuration parameters.
     * @param format The format of request/response body (defaults to JSON).
     * @throw SeverError if cannot connect to the server.
     */
    Sf1DistributedDriver(const std::string& hosts, const Sf1Config& parameters, 
              const Format& format = JSON);
    
    /// Destructor.
    ~Sf1DistributedDriver();
    
private:
    
    /// Perform lazy initialization here.
    void beforeAcquire();
    
    /// Acquire a connection from the ZooKeeper router.
    RawClient& acquire(const std::string& collection) const;
    
    /// Release a connection to the ZooKeeper router.
    void release(const RawClient& connection) const;
    
private:
    
    /// ZooKeeper servers.
    const std::string hosts;
    
    /// The ZooKeeper router.
    boost::scoped_ptr<ZooKeeperRouter> router;
};


NS_IZENELIB_SF1R_END


#endif	/* SF1DISTRIBUTEDDRIVER_HPP */
