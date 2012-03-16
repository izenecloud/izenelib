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
#include <string>


NS_IZENELIB_SF1R_BEGIN

namespace ba = boost::asio;


/**
 * Alias for response objects.
 * A reponse is a pair (sequence, body).
 */
typedef boost::tuple<uint32_t, std::string> Response;

/**
 * Enumeration for \ref Response fields.
 */
enum {
    RESPONSE_SEQUENCE,  ///< The request sequence number
    RESPONSE_BODY       ///< The request body
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
     * @param service A reference to the IO service.
     * @param iterator A reference to the endpoint iterator.
     * @param id An ID for this instance (optional).
     * @throw boost::system::system_error if cannot connect.
     */
    RawClient(ba::io_service& service, 
              ba::ip::tcp::resolver::iterator& iterator,
              const std::string& id = "");
    
    /// Destructor. Must not throw any exception.
    ~RawClient() throw();
    
    /**
     * Checks the connection status.
     * @return \c true if connected, \c false otherwise.
     */
    bool isConnected() const {
        return socket.is_open();
    }

    /**
     * Reads the client status.
     * @return The current status.
     * @see Status
     */
    Status getStatus() const {
        return status;
    }
    
    /**
     * @return The current status as a string.
     * @see Status
     */
    std::string getStatusString() const {
        switch(status) {
        case Idle: return "Idle";
        case Busy: return "Busy";
        case Invalid: return "Invalid";
        }
    }
    
    /**
     * Checks if the client is idle.
     * @return true if the status is \c Idle.
     * @see Status
     */
    bool idle() const {
        return status == Idle;
    }
    
    /**
     * Checks if the client is valid.
     * @return true if the status is not \c Invalid.
     * @see Status
     */
    bool valid() const {
        return status != Invalid;
    }
    
    /**
     * Get the ZooKeeper path associated with this client, if any.
     * @return The ZooKeeper path associated to this client, 
     *         empty if not defined.
     */
    std::string getPath() const {
        return path;
    }
    
    /**
     * Get the ID of this client;
     * @return The ID of this client.
     */
    const uint32_t getId() const {
        return id;
    }

    /**
     * Send a request to SF1.
     * @param sequence request sequence number.
     * @param data request data.
     * @throw std::runtime_error if errors occur.
     * @throw boost::system::system_error if network-related errors occur.
     */
    void sendRequest(const uint32_t& sequence, const std::string& data);
    
    /**
     * Get a response from SF1.
     * @returns the \ref Response containing the sequence number of the 
     *          corresponding request and the response body.
     * @throw std::runtime_error if errors occur.
     * @throw boost::system::system_error if network-related errors occur.
     */
    Response getResponse();
    
private:
    
    /// Socket.
    ba::ip::tcp::socket socket;
    
    /// Current status.
    Status status;
    
    /// ZooKeeper path.
    std::string path;
    
    /// ID number.
    const uint32_t id;
    
    /// Sequence ID;
    static uint32_t idSequence;
};


NS_IZENELIB_SF1R_END

#endif	/* RAWCLIENT_HPP */
