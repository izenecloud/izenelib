/* 
 * File:   Sf1Driver.hpp
 * Author: Paolo D'Apice
 *
 * Created on January 10, 2012, 10:07 AM
 */

#ifndef SF1DRIVER_HPP
#define	SF1DRIVER_HPP

#include "types.h"
#include <boost/noncopyable.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/scoped_ptr.hpp>
#include <stdexcept>
#include <string>


namespace ba = boost::asio;


namespace izenelib {
namespace net {
namespace sf1r {
    

class RawClient;
class Writer;


/**
 * Exception thrown by the Sf1Driver on server errors.
 * Server errors occur when:
 * - SF1 not reachable
 * - SF1 errors (e.g. unmatched sequence number, malformed response)
 */
class ServerError : public std::runtime_error {
public:
    ServerError(const std::string& m = "") : std::runtime_error(m) {}
};


/** 
 * Exception thrown by the Sf1Driver on client errors.
 * Client errors occur when:
 * - missing or wrong URI in request
 * - malformed request
 */
class ClientError : public std::runtime_error {
public:
    ClientError(const std::string& m = "") : std::runtime_error(m) {}
};


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
     * @param port TCP port on which the SF1 is listening (defaults to 18181).
     * @param format the format of request/response body (defaults to JSON).
     * @throw SeverError if cannot connect to the server
     */
    Sf1Driver(const std::string& host, const uint32_t& port = 18181,
        const Format& format = JSON) throw(ServerError);
    
    /// Default constructor (needed for C compatibility).
    Sf1Driver();
    
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
     * @param uri the requested URI.
     * @param tokens 
     * @param request the request body.
     * @throws ClientError errors due to client request occur
     * @throws ServerError errors due to server response occur
     * @return the response body.
     */ 
    std::string call(const std::string& uri, const std::string& tokens,
        std::string& request) throw(ClientError, ServerError);
    
    /**
     * Get the sequence number of the next request.
     * Note that if the request is discarded due to client error
     * (i.e. \ref call throws \ref ClientError, the sequence number
     * is not incremented because no request has been sent to the server.
     */
    uint32_t getSequence() const {
        return sequence;
    }
    
private:
    /// Set data format used for request and responses.
    void setFormat(const Format& format);
    
    uint32_t sequence;
    
    ba::io_service service;
    ba::ip::tcp::resolver resolver;
    ba::ip::tcp::resolver::iterator iterator;
    ba::ip::tcp::resolver::query query;
    
    boost::scoped_ptr<Writer> writer;
    RawClient* client;
    
    // TODO: pool
    
};


}}} /* namespace izenelib::net::sf1r */


#endif	/* SF1DRIVER_HPP */
