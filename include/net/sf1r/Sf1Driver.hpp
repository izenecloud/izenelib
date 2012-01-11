/* 
 * File:   Sf1Driver.hpp
 * Author: Paolo D'Apice
 *
 * Created on January 10, 2012, 10:07 AM
 */

#ifndef SF1DRIVER_HPP
#define	SF1DRIVER_HPP

#include "types.h"
#include <exception>
#include <string>


namespace izenelib {
namespace net {
namespace sf1r {
    

class RawClient;
class Writer;


/// Exception thrown by the Sf1Driver.
class ServerError : public std::exception {
public:
    ServerError(const std::string& m = "") throw() {}
    ~ServerError() throw() {}
};


/**
 * SF1 driver.
 */
class Sf1Driver {
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
     * @return the response body.
     */ 
    std::string call(const std::string& uri, const std::string& tokens,
        std::string& request) throw(ServerError);
    
    /// Get the sequence number of the next request.
    uint32_t getSequence() const {
        return sequence;
    }
    
private:
    // TODO: autoconnect
    
    // Disable copy
    Sf1Driver(const Sf1Driver&);
    void operator=(const Sf1Driver&);
    
    /// Set data format used for request and responses.
    void setFormat(const Format& format);
    
    RawClient* client;
    Writer* writer;
    uint32_t sequence;
};


}}} /* namespace izenelib::net::sf1r */


#endif	/* SF1DRIVER_HPP */
