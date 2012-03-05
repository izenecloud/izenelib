/* 
 * File:   Sf1DriverBase.hpp
 * Author: Paolo D'Apice
 *
 * Created on March 1, 2012, 10:23 AM
 */

#ifndef SF1DRIVER_BASE_HPP
#define	SF1DRIVER_BASE_HPP

#include "config.h"
#include "Errors.hpp"
#include "Sf1Config.hpp"
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>


NS_IZENELIB_SF1R_BEGIN
    
namespace ba = boost::asio;

class PoolFactory;
class RawClient;
class Writer;


/**
 * Base class for SF1 driver.
 */
class Sf1DriverBase : private boost::noncopyable {
public:

    /// Available data formats for request/response body.
    enum Format {
        JSON
    };
    
    /**
     * Creates a new instance of the driver and connects to the server. 
     * @param parameters configuration parameters.
     * @param format The format of request/response body (defaults to JSON).
     */
    Sf1DriverBase(const Sf1Config& parameters, const Format& format = JSON);
    
    /// Destructor.
    virtual ~Sf1DriverBase();
    
    /**
     * Sends a synchronous request to a running SF1 and get the response.
     * The format of both the request and the response is the one defined
     * in the constructor.
     * Usage example:
     * \code
     * Sf1Driver driver(host, port);
     * std::string response = driver.call(request);
     * \endcode
     * @param uri Requested URI.
     * @param tokens Custom tokens.
     * @param request Request body.
     * @throw ClientError if errors due to client request occur.
     * @throw ServerError if errors due to server response occur.
     * @throw ConnectionPoolError if there is no connection available.
     * @return The response body.
     */ 
    std::string call(const std::string& uri, const std::string& tokens,
        std::string& request) throw(ClientError, ServerError, ConnectionPoolError);
    
    /**
     * @return The sequence number of the next request.
     * Note that if the request is discarded due to client error
     * (i.e. \ref call throws \ref ClientError, the sequence number
     * is not incremented because no request has been sent to the server.
     */
    uint32_t getSequence() const {
        return sequence;
    }

    /**
     * @return The actual data format.
     */
    Format getFormat() const {
        return format;
    }
    
    /**
     * @return The actual data format (string).
     */
    std::string getFormatString() const {
        switch (format) {
        case JSON:
        default:
            return "JSON";
    }
}
    
protected:
    
    /// Set data format used for request and responses.
    void setFormat();
    
    /// Acquire a connection to the SF1 according to the given collection.
    virtual RawClient& acquire(const std::string& collection) const = 0;
    
    /// Release the given connection.
    virtual void release(const RawClient& connection) const = 0;
    
protected:
    
    /// Input/Output service.
    ba::io_service service;
    /// TCP host resolver.
    ba::ip::tcp::resolver resolver;
    
    /// Request sequence number.
    uint32_t sequence;
    
    /// Actual driver configuration.
    Sf1Config config;
    
    /// Actual request/response format.
    Format format;
    
    /// Pointer to the actual request/response format handler.
    boost::scoped_ptr<Writer> writer;
    
    /// ConnectionPool factory.
    boost::scoped_ptr<PoolFactory> factory;
};


NS_IZENELIB_SF1R_END


#endif	/* SF1DRIVSF1DRIVER_BASE_HPP */
