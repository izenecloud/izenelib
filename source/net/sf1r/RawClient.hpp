/* 
 * File:   RawClient.hpp
 * Author: Paolo D'Apice
 *
 * Created on December 31, 2011, 2:46 PM
 */

#ifndef RAWCLIENT_HPP
#define	RAWCLIENT_HPP


#include <boost/asio.hpp> 
#include <exception>
#include <string>
#include <utility>


namespace izenelib {
namespace net {
namespace sf1r {


/**
 * Client of the SF1 Driver.
 * Sends raw requests and get raw responses.
 */
class RawClient {
public:
    
    /**
     * Create the driver client.
     * @param host Host of the SF1 Driver Server.
     * @param port Port of the SF1 Driver Server.
     */
    RawClient(const std::string& host, const std::string& port);
    
    /// Destructor.
    virtual ~RawClient();
    
    /// Return the string host:port.
    std::string serverName() const {
        return host + ":" + port;
    }
    
    /**
     * Connect to the SF1 Server.
     * @throw std::exception if cannot connect.
     */
    void connect() throw(std::exception);
    
    /**
     * Close connection to the SF1 Server.
     */
    void close();
    
    /**
     * Reconnect to the SF1 Server.
     * Actually it is an alias for \ref close and \ref connect.
     */
    void reconnect() {
        close();
        connect();
    }
    
    /**
     * Check the connection status.
     * @return true if connected, false otherwise.
     */
    bool isConnected() const {
        return socket != NULL;
    }
    
    /**
     * Send a request to SF1.
     * @param sequence request sequence number.
     * @param data request data.
     * @throw std::exception errors occur
     */
    void sendRequest(const unsigned& sequence, const std::string& data)
    throw(std::exception);
    
    /**
     * Get a response from SF1.
     * @returns a std::pair containg the sequence number of the corresponding 
     *          request and the response
     * @throw std::exception errors occur
     */
    std::pair<unsigned, std::string> getResponse()
    throw(std::exception);
    
private:
    
    // Disable copy
    RawClient(const RawClient&);
    void operator=(const RawClient&);
    
    /// Host of the SF1 Driver Server (BA or proxy)
    std::string host;
    
    /// Port of the SF1 Driver Server (BA or Proxy)
    std::string port;
    
    // Asio Socket
    boost::asio::io_service service;
    boost::asio::ip::tcp::resolver* resolver;
    boost::asio::ip::tcp::resolver::query* query;
    boost::asio::ip::tcp::socket* socket;
};


}}} /* namespace izenelib::net::sf1r */

#endif	/* RAWCLIENT_HPP */
