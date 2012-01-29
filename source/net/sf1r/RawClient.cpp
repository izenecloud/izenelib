/* 
 * File:   RawClient.cpp
 * Author: Paolo D'Apice
 * 
 * Created on December 31, 2011, 2:46 PM
 */

#include "RawClient.hpp"
#include <glog/logging.h>
#include <iostream>


namespace izenelib {
namespace net {
namespace sf1r {


namespace ba = boost::asio;
using ba::ip::tcp;
using std::string;


static const size_t UINT_SIZE = sizeof(uint32_t);

/**
 * Header size, i.e. two unsigned values for:
 * - sequence number
 * - message length
 */
static const size_t HEADER_SIZE = 2 * UINT_SIZE;


RawClient::RawClient(const string& h, const string& p) 
        : host(h), port(p)  {
    resolver = new tcp::resolver(service);
    query = new tcp::resolver::query(/*tcp::v4(),*/ host, port); // TODO: restrict to v4?
    socket = NULL;
    
    DLOG(INFO) << "instantiated";
}


RawClient::~RawClient() {
    delete socket;
    delete query;
    delete resolver;
    
    DLOG(INFO) << "destroyed";
}


void
RawClient::connect() throw (std::exception) {
    try {
        DLOG(INFO) << "connecting to " << serverName() << " ...";
        
        socket = new tcp::socket(service);
        ba::connect(*socket, resolver->resolve(*query)); 
        
        DLOG(INFO) << "connected";
    } catch (std::exception& e) {
        string message = "Unable to connect to " + serverName();
        LOG(ERROR) << message << ": " << e.what();
        throw std::runtime_error(message);
    }
}


void
RawClient::close() {
    try {
        DLOG(INFO) << "closing ...";
    
        socket->shutdown(socket->shutdown_both);
        socket->close();
        socket = NULL;
    
        DLOG(INFO) << "connection closed";
    } catch (std::exception& e) {
        LOG(WARNING) << e.what();
    }
}


void
RawClient::sendRequest(const uint32_t& sequence, const string& data)
throw (std::exception) {
    if (!isConnected()) {
        throw std::runtime_error("Not connected");
    }
    
    DLOG(INFO) << "sending request ...";
    
    DLOG(INFO) << "request: [" 
               << sequence << ", " 
               << data.length() << ", "
               << data << "]";
    
    uint32_t seq = htonl(sequence);
    uint32_t len = htonl(data.length());
    
    std::vector<ba::const_buffer> buffers;
    buffers.push_back(ba::buffer(&seq, UINT_SIZE));
    buffers.push_back(ba::buffer(&len, UINT_SIZE));
    buffers.push_back(ba::buffer(data));

    size_t n = ba::write(*socket, buffers);
    if (n != HEADER_SIZE + data.length()) {
        const string message = "Connection to " + serverName() + "lost";
        throw std::runtime_error(message);
    }   
    
    LOG(INFO) << "request sent (" << n << " bytes)";
}


std::pair<uint32_t, string>
RawClient::getResponse() throw (std::exception) {
    if (!isConnected()) {
        throw std::runtime_error("Not connected");
    }
    
    LOG(INFO) << "receiving response ...";

    char header[HEADER_SIZE];
    size_t n = 0;

    n += ba::read(*socket, ba::buffer(header));
    if (n != sizeof(HEADER_SIZE)) {
        string message = "Connection to " + serverName() + "lost";
        throw std::runtime_error(message);
    }

    uint32_t sequence, length;
    memcpy(&sequence, header, UINT_SIZE);
    memcpy(&length, header + UINT_SIZE, UINT_SIZE);

    sequence = ntohl(sequence);
    length = ntohl(length);

    DLOG(INFO) << "sequence\t:" << sequence;
    DLOG(INFO) << "length\t:" << length;

    char data[length];
    n = ba::read(*socket, ba::buffer(data, length));
    if (n != length) {
        string message = "Connection to " + serverName() + "lost";
        throw std::runtime_error(message);
    }

    string response(data, length - 1); // skip the final '\n'
    DLOG(INFO) << "data\t:[" << response << "]";

    LOG(INFO) << "response received (" << n + HEADER_SIZE << " bytes)";
    
    return std::make_pair<uint32_t, string>(sequence, response);
}


}}} /* namespace izenelib::net::sf1r */
