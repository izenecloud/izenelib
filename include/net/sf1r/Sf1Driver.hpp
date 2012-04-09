/* 
 * File:   Sf1Driver.hpp
 * Author: Paolo D'Apice
 *
 * Created on January 10, 2012, 10:07 AM
 */

#ifndef SF1DRIVER_HPP
#define	SF1DRIVER_HPP

#include "Sf1DriverBase.hpp"
#include <boost/scoped_ptr.hpp>


NS_IZENELIB_SF1R_BEGIN
    

class ConnectionPool;


/**
 * SF1 driver.
 * This class connects to a <b>single</b> SF1.
 */
class Sf1Driver : public Sf1DriverBase {
public:

    /**
     * Creates a new instance of the driver.
     * @param host The address of the SF1 server in the format "host:port".
     * @param parameters The configuration parameters.
     * @param format The format of request/response body (defaults to JSON).
     */
    Sf1Driver(const std::string& host, const Sf1Config& parameters, 
              const Format& format = JSON);
    
    /// Destructor.
    ~Sf1Driver();
    
    /**
     * Sends a synchronous request to a running SF1 and get the response.
     * @see Sf1DriverBase.call()
     */
    std::string call(const std::string& uri, const std::string& tokens,
                     std::string& request);

    /**
     * @return The actual pool size.
     */
    size_t getPoolSize() const;
    
private:
    
    /// Acquire a connection from the connection pool.
    RawClient& acquire(const std::string& collection) const;
    
    /// Release a connection into the connection pool.
    void release(const RawClient& connection) const;
    
    /// Reconnect to the SF1.
    void reconnect();
    
private:
    
    /// Address of the target SF1.
    const std::string host;
    
    /// Flag indicating whether must reconnect before a request.
    bool mustReconnect;
    
    /// Connection pool to the SF1.
    boost::scoped_ptr<ConnectionPool> pool;
};


NS_IZENELIB_SF1R_END


#endif	/* SF1DRIVER_HPP */
