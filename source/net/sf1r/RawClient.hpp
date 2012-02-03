/* 
 * File:   RawClient.hpp
 * Author: Paolo D'Apice
 *
 * Created on December 31, 2011, 2:46 PM
 */

#ifndef RAWCLIENT_HPP
#define	RAWCLIENT_HPP


#include "net/sf1r/config.h"
#include "types.h"
#include <boost/asio.hpp> 
#include <boost/tuple/tuple.hpp>
#include <exception>
#include <string>


namespace ba = boost::asio;


NS_IZENELIB_SF1R_BEGIN

/**
 * Alias for response objects.
 */
typedef boost::tuple<uint32_t, std::string> Response;

/**
 * Enumeration for \ref Response fields.
 */
enum {
    RESPONSE_SEQUENCE,
    RESPONSE_BODY
};


/**
 * Client of the SF1 Driver.
 * Sends requests and get responses using the custom SF1 protocol:
 * - header
 * -- sequence number (4 bytes)
 * -- body length (4 bytes)
 * - body
 */
class RawClient : private boost::noncopyable {
public:
    
    /**
     * Client status.
     */
    enum Status {
        Idle,           ///< Waiting for operation
        Busy,           ///< Performing send/receive
        Invalid         ///< Connection error occurred
    };

    /**
     * Creates the driver client.
     * @param service a reference to the IO service.
     * @param iterator a reference to the endpoint iterator.
     * @throw boost::system::system_error if cannot connect.
     */
    RawClient(ba::io_service& service, 
              ba::ip::tcp::resolver::iterator& iterator);
    
    /// Destructor. Must not throw any exception.
    ~RawClient() throw();
    
    // TODO: keepalive
    
    /**
     * Checks the connection status.
     * @return true if connected, false otherwise.
     */
    bool isConnected() const {
        return socket.is_open();
    }

    /**
     * Reads the client status.
     * \ref Status
     */
    Status getStatus() const {
        return status;
    }
    
    /**
     * Checks if the client is idle.
     * \ref Status
     */
    bool idle() const {
        return status == Idle;
    }

    /**
     * Send a request to SF1.
     * @param sequence request sequence number.
     * @param data request data.
     * @throw std::exception if errors occur.
     */
    void sendRequest(const uint32_t& sequence, const std::string& data)
    throw(std::exception);
    
    /**
     * Get a response from SF1.
     * @returns the \ref Response containing the sequence number of the 
     *          corresponding request and the response body.
     * @throw std::exception if errors occur.
     */
    Response getResponse()
    throw(std::exception);
    
private:

    ba::ip::tcp::socket socket;
    Status status;
    
};


NS_IZENELIB_SF1R_END

#endif	/* RAWCLIENT_HPP */
