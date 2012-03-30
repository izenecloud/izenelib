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

using boost::system::system_error;
using ba::ip::tcp;
using std::string;
using std::runtime_error;


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


uint32_t 
RawClient::idSequence = 0;


RawClient::RawClient(ba::io_service& service, 
                     const std::string& host, const std::string& port,
                     const string& zkpath) 
        : socket(service), status(Idle), path(zkpath), id(++idSequence) {
    try {
        DLOG(INFO) << "connecting (" << id << ") ...";
        
        ba::ip::tcp::resolver resolver(service);
        ba::ip::tcp::resolver::query query(host, port);
        
        ba::connect(socket, resolver.resolve(query)); 
        
        DLOG(INFO) << "connected (" << id << ")";
    } catch (system_error& e) {
        status = Invalid;
        LOG(ERROR) << e.what();
        throw e;
    }

    CHECK_EQ(Idle, status) << "not Idle (" << id << ")";
    DLOG(INFO) << "Correctly instantiated (" << id << ")";
}


RawClient::~RawClient() {
    CHECK_NE(Busy, status);
    try {
        DLOG(INFO) << "closing (" << id << ") ...";
        
        socket.shutdown(socket.shutdown_both);
        socket.close();
        
        DLOG(INFO) << "connection closed (" << id << ")";
    } catch (system_error& e) {
        LOG(WARNING) << e.what();
    }
    
    DLOG(INFO) << "Correctly destroyed (" << id << ")";
}


void
RawClient::sendRequest(const uint32_t& sequence, const string& data) {
    if (not isConnected()) {
        // TODO: keep alive?
        status = Invalid;
        throw runtime_error("Not connected");
    }
    
    CHECK_EQ(Idle, status) << "not Idle (" << id << ")";
    status = Busy;
    
    DLOG(INFO) << "Sending raw request (" << id << "): ["
               << sequence << ", " 
               << data.length() << ", "
               << data << "] ...";
    
    uint32_t seq = htonl(sequence);
    uint32_t len = htonl(data.length());
    
    boost::array<ba::const_buffer,3> buffers;
    buffers[0] = ba::buffer(&seq, UINT_SIZE);
    buffers[1] = ba::buffer(&len, UINT_SIZE);
    buffers[2] = ba::buffer(data);

    size_t n = 0;
    try {
        n += ba::write(socket, buffers);
    } catch (system_error& e) {
        status = Invalid;
        LOG(ERROR) << e.what();
        throw e;
    }
    
    if (n != HEADER_SIZE + data.length()) {
        status = Invalid;
        throw runtime_error("write: Write size mismatch");
    }
    
    // do not change the status
    CHECK_EQ(Busy, status) << "not Busy (" << id << ")";
    DLOG(INFO) << "Request sent (" << n << " bytes).";
}


Response
RawClient::getResponse() {
    if (not isConnected()) {
        // TODO: keep alive?
        status = Invalid;
        throw runtime_error("Not connected");
    }
    
    CHECK_EQ(Busy, status) << "not Busy (" << id << ")";
    // do not change the status
    DLOG(INFO) << "Receiving response (" << id << ") ...";

    char header[HEADER_SIZE];
    size_t n = 0;

    try {
        n += ba::read(socket, ba::buffer(header));
    } catch (system_error& e) {
        status = Invalid;
        LOG(ERROR) << e.what();
        throw e;
    }
    
    if (n != sizeof(HEADER_SIZE)) {
        status = Invalid;
        throw runtime_error("read: Read size mismatch");
    }

    uint32_t sequence, length;
    memcpy(&sequence, header, UINT_SIZE);
    memcpy(&length, header + UINT_SIZE, UINT_SIZE);

    sequence = ntohl(sequence);
    length = ntohl(length);

    char data[length];
    try {
        n = ba::read(socket, ba::buffer(data, length));
    } catch (system_error& e) {
        status = Invalid;
        LOG(ERROR) << e.what();
        throw e;
    }
    
    if (n != length) {
        status = Invalid;
        throw runtime_error("read: Read size mismatch");
    }

    string response(data, length - 1); // skip the final '\0'
    DLOG(INFO) << "Response (" << id << "): ["
               << sequence << ", "
               << length << ", "
               << response << "]";

    LOG(INFO) << "Response received (" << n + HEADER_SIZE << " bytes)";
    
    status = Idle;
    return boost::make_tuple(sequence, response);
}


NS_IZENELIB_SF1R_END
