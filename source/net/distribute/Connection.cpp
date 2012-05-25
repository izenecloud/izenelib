/* 
 * File:   Connection.cpp
 * Author: paolo
 * 
 * Created on May 24, 2012, 3:43 PM
 */

#include "net/distribute/Connection.hpp"

#include <boost/bind.hpp>
#include <glog/logging.h>

using net::distribute::SendFileReqMsg;
using net::distribute::ResponseMsg;

using ba::ip::tcp;
using std::ofstream;
using std::string;

NS_IZENELIB_DISTRIBUTE_BEGIN

static unsigned i = 0;

#define ID "(" << id << ") "

Connection::Connection(ba::io_service& service, const size_t& size) 
        : id(i++), socket(service), bufferSize(size), basedir(DEFAULT_DESTINATION) {
    // TODO: use class FileWriter
    buffer = new char[bufferSize];
    DLOG(INFO) << ID << "connection instantiated";
}


Connection::~Connection() {
    delete[] buffer;
    DLOG(INFO) << ID << "connection destroyed";
}


void
Connection::start() {
    DLOG(INFO) << ID << "starting ";
    
    // read request header
    socket.async_read_some(ba::buffer(buffer, bufferSize),
            boost::bind(&Connection::handleReceivedHeader, this,
                        ba::placeholders::error,
                        ba::placeholders::bytes_transferred));
}


void
Connection::handleReceivedHeader(const bs::error_code& error, size_t size) {
    DLOG(INFO) << ID << "received " << size << " bytes";
    if (error) {
        LOG(ERROR) << ID << error.message();
        return;
    }
    if (size <= 0) {
        LOG(ERROR) << ID << "No data received";
        return;
    }
    DLOG(INFO) << ID << "header: " << string(buffer, size);
    header.loadMsg(string(buffer, size));
    
    // TODO: create file
    bool flag = createFile(header.getFileName());
    
    ResponseMsg headerAck;
    headerAck.setStatus(flag ? "failed" : "success");
    string s(headerAck.toString());
    
    // TODO: send ack
    ba::async_write(socket, ba::buffer(s, s.length()),
            boost::bind(&Connection::handleSentHeaderAck, this,
                        ba::placeholders::error,
                        ba::placeholders::bytes_transferred));
}


void
Connection::handleSentHeaderAck(const bs::error_code& error, size_t size) {
    DLOG(INFO) << ID << "sent " << size << " bytes";
    if (error) {
        LOG(ERROR) << error.message();
        return;
    }
    
    // TODO: receive file using stream or plain socket (as in DataTransfer2)
    
    // TODO: use handleReceiveFileData callback
    DLOG(INFO) << ID << "header ack correctly sent";
    
}


void
Connection::handleReceivedFileData(const bs::error_code& error, size_t size) {
    DLOG(WARNING) << ID << "TODO: receive file data";
}


void 
Connection::handleSentFileDataAck(const bs::error_code& error, size_t size) {
    DLOG(WARNING) << ID << "TODO: send file data ack";
}


bool
Connection::createFile(const string& fileName) {
    bfs::path filepath = basedir / bfs::path(fileName);
    DLOG(INFO) << ID << "filepath: " << filepath;
    
    // FIXME: completare
    return true;
}


NS_IZENELIB_DISTRIBUTE_END
