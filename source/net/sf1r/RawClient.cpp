/* 
 * File:   RawClient.cpp
 * Author: Paolo D'Apice
 * 
 * Created on December 31, 2011, 2:46 PM
 */

#include "net/sf1r/Errors.hpp"
#include "RawClient.hpp"
#include <boost/array.hpp>
#include <glog/logging.h>
#include <sys/time.h>

NS_IZENELIB_SF1R_BEGIN

namespace bs = boost::system;

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


uint32_t 
RawClient::idSequence = 0;


RawClient::RawClient(ba::io_service& service, 
                     const std::string& host, const std::string& port,
                     const size_t timeout, const string& zkpath) 
        : socket(service), status(Idle), path(zkpath), id(++idSequence) {
    try {
        DLOG(INFO) << "configuring socket ...";
        // TODO: windows?
        struct timeval tv;
        tv.tv_sec = timeout;
        tv.tv_usec = 0;
        setsockopt(socket.native(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof (tv));
        setsockopt(socket.native(), SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof (tv));
        
        DLOG(INFO) << "connecting to [" << host << ":" << port << "] (" << id << ") ...";
        
        ba::ip::tcp::resolver resolver(service);
        ba::ip::tcp::resolver::query query(host, port);
        
        ba::connect(socket, resolver.resolve(query)); 
        
        DLOG(INFO) << "connected (" << id << ")";
    } catch (bs::system_error& e) {
        status = Invalid;
        LOG(ERROR) << e.what();
        throw NetworkError(e.what());
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
    } catch (bs::system_error& e) {
        LOG(WARNING) << e.what();
    }
    
    DLOG(INFO) << "Correctly destroyed (" << id << ")";
}


bool
RawClient::isConnected() {
    // is the socket open?
    if (not socket.is_open()) {
        DLOG(WARNING) << "Socket is not open";
        return false;
    }
    
    // is the endpoint active?
    try {
        socket.remote_endpoint();
    } catch (bs::system_error& e) {
        DLOG(WARNING) << "Endpoint error: " << e.what();
        return false;
    }
    
    return true;
}


void
RawClient::sendRequest(const uint32_t& sequence, const string& data) {
    if (not isConnected()) {
        // TODO: keep alive?
        status = Invalid;
        throw NetworkError("Not connected");
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
    } catch (bs::system_error& e) {
        status = Invalid;
        LOG(ERROR) << e.what();
        throw NetworkError(e.what());
    }
    
    if (n != HEADER_SIZE + data.length()) {
        status = Invalid;
        throw ServerError("write: Write size mismatch");
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
        throw NetworkError("Not connected");
    }
    
    CHECK_EQ(Busy, status) << "not Busy (" << id << ")";
    // do not change the status
    DLOG(INFO) << "Receiving response (" << id << ") ...";

    char header[HEADER_SIZE];
    size_t n = 0;

    try {
        n += ba::read(socket, ba::buffer(header));
    } catch (bs::system_error& e) {
        status = Invalid;
        LOG(ERROR) << e.what();
        throw NetworkError(e.what());
    }
    
    if (n != sizeof(HEADER_SIZE)) {
        status = Invalid;
        throw ServerError("Read size mismatch");
    }

    uint32_t sequence, length;
    memcpy(&sequence, header, UINT_SIZE);
    memcpy(&length, header + UINT_SIZE, UINT_SIZE);

    sequence = ntohl(sequence);
    length = ntohl(length);

    char data[length];
    try {
        n = ba::read(socket, ba::buffer(data, length));
    } catch (bs::system_error& e) {
        status = Invalid;
        LOG(ERROR) << e.what();
        throw NetworkError(e.what());
    }
    
    if (n != length) {
        status = Invalid;
        throw ServerError("Read size mismatch");
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
