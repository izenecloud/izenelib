/* 
 * File:   Sf1Driver.hpp
 * Author: Paolo D'Apice
 *
 * Created on January 10, 2012, 10:07 AM
 */

#ifndef SF1DRIVER_HPP
#define	SF1DRIVER_HPP

#include "Sf1DriverBase.hpp"
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/scoped_ptr.hpp>


NS_IZENELIB_SF1R_BEGIN
    

class ConnectionPool;


/**
 * SF1 driver.
 */
class Sf1Driver : public Sf1DriverBase {
public:

    /**
     * Creates a new instance of the driver and connects to the server. 
     * @param host hostname or IP address of the SF1 server.
     * @param port TCP port on which the SF1 is listening.
     * @param parameters configuration parameters.
     * @param format The format of request/response body (defaults to JSON).
     * @throw SeverError if cannot connect to the server.
     */
    Sf1Driver(const std::string& host, const uint32_t& port,
              const Sf1Config& parameters, 
              const Format& format = JSON) throw(ServerError);
    
    /// Destructor.
    ~Sf1Driver();
    
    /**
     * Sends a synchronous request to the SF1 server and get the response.
     */ 
    std::string call(const std::string& uri, const std::string& tokens,
        std::string& request) throw(ClientError, ServerError, ConnectionPoolError);
    
    /**
     * @return The actual pool size.
     */
    size_t getPoolSize() const;
    
private:
    
    boost::scoped_ptr<ConnectionPool> pool;
};


NS_IZENELIB_SF1R_END


#endif	/* SF1DRIVER_HPP */
