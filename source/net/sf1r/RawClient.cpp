/* 
 * File:   RawClient.cpp
 * Author: Paolo D'Apice
 * 
 * Created on December 31, 2011, 2:46 PM
 */

#include "RawClient.hpp"
#include <boost/array.hpp>
#include <glog/logging.h>


NS_IZENELIB_SF1R_BEGIN


using ba::ip::tcp;
using std::string;


namespace {

/**
 * Size of unsigned integer (32 bits).
 */
const size_t UINT_SIZE = sizeof(uint32_t);

/**
 * Header size, i.e. two unsigned values for:
 * - sequence number
 * - message length
 */
const size_t HEADER_SIZE = 2 * UINT_SIZE;

}


RawClient::RawClient(ba::io_service& service, 
                     tcp::resolver::iterator& iterator,
                     const string& zkpath) 
        : socket(service), status(Idle), id(zkpath) {
    try {
        DLOG(INFO) << "connecting ...";
        ba::connect(socket, iterator); 
        
        DLOG(INFO) << "connected";
    } catch (boost::system::system_error& e) {
        status = Invalid;
        LOG(ERROR) << e.what();
        throw e;
    }

    CHECK_EQ(Idle, status) << "not Idle";
    DLOG(INFO) << "Correctly instantiated";
}


RawClient::~RawClient() throw() {
    CHECK_EQ(Idle, status) << "not Idle";
    try {
        DLOG(INFO) << "closing ...";
        
        socket.shutdown(socket.shutdown_both);
        socket.close();
        
        DLOG(INFO) << "connection closed";
    } catch (boost::system::system_error& e) {
        LOG(WARNING) << "WARNING: " << e.what();
    }
    
    DLOG(INFO) << "Correctly destroyed.";
}


void
RawClient::sendRequest(const uint32_t& sequence, const string& data)
throw (std::exception) {
    if (not isConnected()) {
        // TODO: keep alive?
        status = Invalid;
        throw std::runtime_error("Not connected");
    }
    
    CHECK_EQ(Idle, status) << "not Idle";
    status = Busy;
    
    DLOG(INFO) << "Sending raw request ["
               << sequence << ", " 
               << data.length() << ", "
               << data << "] ...";
    
    uint32_t seq = htonl(sequence);
    uint32_t len = htonl(data.length());
    
    boost::array<ba::const_buffer,3> buffers;
    buffers[0] = ba::buffer(&seq, UINT_SIZE);
    buffers[1] = ba::buffer(&len, UINT_SIZE);
    buffers[2] = ba::buffer(data);

    size_t n = ba::write(socket, buffers);
    if (n != HEADER_SIZE + data.length()) {
        status = Invalid;
        const string message = "Connection lost";
        throw std::runtime_error(message);
    }   
    
    // do not change the status
    CHECK_EQ(Busy, status) << "not Busy";
    DLOG(INFO) << "Request sent (" << n << " bytes).";
}


Response
RawClient::getResponse() throw (std::exception) {
    if (not isConnected()) {
        // TODO: keep alive?
        status = Invalid;
        throw std::runtime_error("Not connected");
    }
    
    CHECK_EQ(Busy, status) << "not Busy";
    // do not change the status
    DLOG(INFO) << "Receiving response ...";

    char header[HEADER_SIZE];
    size_t n = 0;

    n += ba::read(socket, ba::buffer(header));
    if (n != sizeof(HEADER_SIZE)) {
        status = Invalid;
        string message = "Connection lost";
        throw std::runtime_error(message);
    }

    uint32_t sequence, length;
    memcpy(&sequence, header, UINT_SIZE);
    memcpy(&length, header + UINT_SIZE, UINT_SIZE);

    sequence = ntohl(sequence);
    length = ntohl(length);

    char data[length];
    n = ba::read(socket, ba::buffer(data, length));
    if (n != length) {
        status = Invalid;
        string message = "Connection to lost";
        throw std::runtime_error(message);
    }

    string response(data, length - 1); // skip the final '\0'
    DLOG(INFO) << "Response: ["
               << sequence << ", "
               << length << ", "
               << response << "]";

    DLOG(INFO) << "Response received (" << n + HEADER_SIZE << " bytes)";
    
    status = Idle;
    return boost::make_tuple(sequence, response);
}


NS_IZENELIB_SF1R_END
