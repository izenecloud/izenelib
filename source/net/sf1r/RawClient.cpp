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


/**
 * Header size, i.e. two unsigned values for:
 * <ul>
 * <li>sequence number</li>
 * <li>message length</li>
 * </ul>
 */
const size_t HEADER_SIZE = 2 * sizeof(unsigned);


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
        LOG(INFO) << "connecting to " << serverName() << " ...";
        
        socket = new tcp::socket(service);
        ba::connect(*socket, resolver->resolve(*query)); 
        
        LOG(INFO) << "connected";
    } catch (std::exception& e) {
        string message = "Unable to connect to " + serverName();
        LOG(ERROR) << message;
        LOG(ERROR) << e.what();
        throw std::runtime_error(message);
    }
}


void
RawClient::close() {
    try {
        LOG(INFO) << "closing ...";
    
        socket->shutdown(socket->shutdown_both);
        socket->close();
        socket = NULL;
    
        LOG(INFO) << "connection closed";
    } catch (std::exception& e) {
        LOG(WARNING) << e.what();
    }
}


void
RawClient::sendRequest(const unsigned& sequence, const string& data)
throw (std::exception) {
    if (!isConnected()) {
        throw std::runtime_error("Not connected");
    }
    
    LOG(INFO) << "sending request ...";
    
    DLOG(INFO) << "sequence\t:" << sequence;
    DLOG(INFO) << "length\t:" << data.length();
    DLOG(INFO) << "data\t:" << data;
    
    unsigned seq = htonl(sequence);         // TODO: boost functions
    unsigned len = htonl(data.length());    // TODO: boost functions
    
    std::vector<ba::const_buffer> buffers;
    buffers.push_back(ba::buffer(&seq, sizeof(seq)));
    buffers.push_back(ba::buffer(&len, sizeof(len)));
    buffers.push_back(ba::buffer(data));

    size_t n = ba::write(*socket, buffers);
    if (n != HEADER_SIZE + data.length()) {
        string message = "Connection to " + serverName() + "lost";
        throw std::runtime_error(message);
    }   
    
    LOG(INFO) << "request sent (" << n << " bytes)";
}


std::pair<unsigned, string>
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

    //warning: dereferencing type-punned pointer will break strict-aliasing rules
    // -fno-strict-aliasing 
    //unsigned sequence = ntohl(*(unsigned*)(header));
    //unsigned length = ntohl(*(unsigned*)(header + sizeof(unsigned)));

    unsigned sequence, length;
    memcpy(&sequence, header, sizeof(unsigned));
    memcpy(&length, header + sizeof(unsigned), sizeof(unsigned));

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
    
    return std::make_pair<unsigned, string>(sequence, response);
}


}}} /* namespace izenelib::net::sf1r */
