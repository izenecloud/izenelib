/* 
 * File:   Sf1Driver.hpp
 * Author: Paolo D'Apice
 *
 * Created on January 10, 2012, 10:07 AM
 */

#ifndef SF1DRIVER_HPP
#define	SF1DRIVER_HPP

#include "config.h"
#include "Errors.hpp"
#include "Sf1Config.hpp"
#include <boost/noncopyable.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/scoped_ptr.hpp>


namespace ba = boost::asio;


NS_IZENELIB_SF1R_BEGIN
    

class ConnectionPool;
class Writer;


/**
 * SF1 driver.
 */
class Sf1Driver : private boost::noncopyable {
public:

    /// Available data formats for request/response body.
    enum Format {
        JSON
    };
    
    /**
     * Creates a new instance of the driver and connects to the server. 
     * @param host hostname or IP address of the SF1 server.
     * @param port TCP port on which the SF1 is listening.
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
     * @return The actual pool size.
     */
    size_t getPoolSize() const;
    
    /**
     * @return The actual data format.
     */
    Format getFormat() const {
        return format;
    }
    
    /**
     * @return The actual data format (string).
     */
    std::string getFormatString() const;
    
private:
    /// Set data format used for request and responses.
    void setFormat();
    
    /// Initializes the connection pool.
    void initPool(const Sf1Config& parameters);
    
    uint32_t sequence;
    
    ba::io_service service;
    ba::ip::tcp::resolver resolver;
    ba::ip::tcp::resolver::iterator iterator;
    ba::ip::tcp::resolver::query query;
    
    ConnectionPool* pool;
    
    Format format;
    boost::scoped_ptr<Writer> writer;
    
};


NS_IZENELIB_SF1R_END


#endif	/* SF1DRIVER_HPP */
