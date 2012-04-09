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

namespace {

/**
 * Max sequence number.
 * Valued 4294967294 as defined in the Ruby client:
 * \code 
 * MAX_SEQUENCE = (1 << 31) - 2
 * \endcode
 * @see \ref limits
 */
const uint32_t MAX_SEQUENCE = std::numeric_limits<uint32_t>::max() - 1;

}

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
     * Initializes the pool factory and the data writer. 
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
     * @param[in] uri Requested URI.
     * @param[in] tokens Custom tokens.
     * @param[in,out] request Request body.
     * @return The response body.
     * @throw ClientError if errors due to client request occur.
     * @throw ConnectionPoolError if there is no connection available.
     * @throw ServerError if errors due to server response occur.
     * @throw NetworkError if cannot connect to the server.
     */ 
    virtual std::string call(const std::string& uri, const std::string& tokens,
                             std::string& request) = 0;
    
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
    
    /* pure virtual methods */
    
    /// Acquire a connection to the SF1 according to the given collection.
    virtual RawClient& acquire(const std::string& collection) const = 0;
    
    /// Release the given connection.
    virtual void release(const RawClient& connection) const = 0;
    
    /* basic methods (private API) */
    
    /**
     * Parse the uri for controller and action.
     * @param[in] uri The request URI.
     * @param[out] controller The request controller.
     * @param[out] action The request action (may be empty).
     * @throw ClientError if the controller is not specified.
     */
    void parseUri(const std::string& uri, std::string& controller, 
                  std::string& action) const;
    
    /**
     * Preprocess the request:
     * - check data format
     * - set request header
     * - extract collection
     * @param[in] controller The request controller.
     * @param[in] action The request action.
     * @param[in] tokens The request tokens.
     * @param[in,out] request The current request.
     * @param[out] collection The request collection.
     * @throw ClientError if the request is not valid.
     */
    void preprocessRequest(const std::string& controller, const std::string& action,
                           const std::string& tokens, std::string& request, 
                           std::string& collection) const;
    
    /// Increment the request sequence number.
    void incrementSequence() {
        if (++sequence == MAX_SEQUENCE) {
            sequence = 1; // sequence == 0 means server error
        }  
    }
    
    /**
     * Send request to and receive response from the SF1.
     * @param[in] connection The current connection to the SF1.
     * @param[in] request The current request.
     * @param[out] response The response to the current request.
     * @throw ServerError if error occur on SF1 while processing the request.
     * @see RawClient
     */
    void sendAndReceive(RawClient& connection, const std::string& request,
                        std::string& response);
    
protected:
    
    friend class Releaser;
    
    /// Set data format used for request and responses.
    void setFormat();
    
    /// Request sequence number.
    uint32_t sequence;
    
    /// Actual request/response format.
    Format format;
    
    /// Input/Output service.
    ba::io_service service;
    
    /// Pointer to the actual request/response format handler.
    boost::scoped_ptr<Writer> writer;
    
    /// ConnectionPool factory.
    boost::scoped_ptr<PoolFactory> factory;
};


NS_IZENELIB_SF1R_END


#endif	/* SF1DRIVSF1DRIVER_BASE_HPP */
