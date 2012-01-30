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


using ba::ip::tcp;
using std::string;


/**
 * Size of unsigned integer (32 bits).
 */
static const size_t UINT_SIZE = sizeof(uint32_t);

/**
 * Header size, i.e. two unsigned values for:
 * - sequence number
 * - message length
 */
static const size_t HEADER_SIZE = 2 * UINT_SIZE;


RawClient::RawClient(ba::io_service& service, 
                     tcp::resolver::iterator& iterator) 
        : socket(service) {
    try {
        DLOG(INFO) << "connecting ...";
        ba::connect(socket, iterator); 
        
        DLOG(INFO) << "connected";
    } catch (boost::system::system_error& e) {
        LOG(ERROR) << e.what();
        throw e;
    }

    DLOG(INFO) << "correctly instantiated";
}


RawClient::~RawClient() {
    try {
        DLOG(INFO) << "closing ...";
    
        socket.shutdown(socket.shutdown_both);
        socket.close();
        
        DLOG(INFO) << "connection closed";
    } catch (boost::system::system_error& e) {
        LOG(WARNING) << e.what();
    }
    
    DLOG(INFO) << "correctly destroyed";
}


void
RawClient::sendRequest(const uint32_t& sequence, const string& data)
throw (std::exception) {
    if (!isConnected()) {
        // TODO: keep alive?
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

    size_t n = ba::write(socket, buffers);
    if (n != HEADER_SIZE + data.length()) {
        const string message = "Connection lost";
        throw std::runtime_error(message);
    }   
    
    LOG(INFO) << "request sent (" << n << " bytes)";
}


std::pair<uint32_t, string>
RawClient::getResponse() throw (std::exception) {
    if (!isConnected()) {
        // TODO: keep alive?
        throw std::runtime_error("Not connected");
    }
    
    DLOG(INFO) << "receiving response ...";

    char header[HEADER_SIZE];
    size_t n = 0;

    n += ba::read(socket, ba::buffer(header));
    if (n != sizeof(HEADER_SIZE)) {
        string message = "Connection lost";
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
    n = ba::read(socket, ba::buffer(data, length));
    if (n != length) {
        string message = "Connection to lost";
        throw std::runtime_error(message);
    }

    string response(data, length - 1); // skip the final '\0'
    DLOG(INFO) << "data\t:[" << response << "]";

    LOG(INFO) << "response received (" << n + HEADER_SIZE << " bytes)";
    
    return std::make_pair<uint32_t, string>(sequence, response);
}


}}} /* namespace izenelib::net::sf1r */
