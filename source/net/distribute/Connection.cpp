/* 
 * File:   Connection.cpp
 * Author: Paolo D'Apice
 * 
 * Created on May 24, 2012, 3:43 PM
 */

#include "net/distribute/Connection.hpp"

#include <boost/bind.hpp>
#include <glog/logging.h>

using ba::ip::tcp;
using std::string;

namespace {
/// Connection ID sequence.
unsigned i = 0;
}

// Get connection ID to be printed in logs.
#define ID "(" << id << ") "


NS_IZENELIB_DISTRIBUTE_BEGIN


Connection::Connection(ba::io_service& service, const size_t& size,
            const string& dir) 
        : id(i++), socket(service), bufferSize(size), basedir(dir) {
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
            boost::bind(&Connection::handleReceivedHeader, shared_from_this(),
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
    
    // deserialize
    msgpack::unpacked unp;
    msgpack::unpack(&unp, buffer, size);
    request = unp.get().as<Request>();
    DLOG(INFO) << ID << "request: " << request;
    fileSize = request.getFileSize(); //XXX server?
    
    RequestAck ack(createFile());
    
    // serialize and send
    msgpack::sbuffer sbuff;
    msgpack::pack(sbuff, ack);
    DLOG(INFO) << ID << "sending ack: " << ack;
    ba::async_write(socket, ba::buffer(sbuff.data(), sbuff.size()),
            boost::bind(&Connection::handleSentHeaderAck, shared_from_this(),
                        ba::placeholders::error,
                        ba::placeholders::bytes_transferred));
}


void
Connection::handleSentHeaderAck(const bs::error_code& error, size_t size) {
    DLOG(INFO) << ID << "sent " << size << " bytes";
    if (error) {
        LOG(ERROR) << ID << error.message();
        return;
    }
    if (size <= 0) {
        LOG(ERROR) << ID << "No data sent";
        return;
    }
    
    bool flag = true;
    size_t receivedSize = receiveFileData();
    if (receivedSize != fileSize) {
        LOG(ERROR) << ID << "Received " << receivedSize << "/" << fileSize << " bytes";
        flag = false;
    }
    
    RequestAck ack(flag, receivedSize);
    
    // serialize and send
    msgpack::sbuffer sbuff;
    msgpack::pack(sbuff, ack);
    DLOG(INFO) << ID << "sending data ack: " << ack;
    ba::async_write(socket, ba::buffer(sbuff.data(), sbuff.size()),
            boost::bind(&Connection::handleSentFileDataAck, shared_from_this(),
                        ba::placeholders::error,
                        ba::placeholders::bytes_transferred));
}


void 
Connection::handleSentFileDataAck(const bs::error_code& error, size_t size) {
    DLOG(INFO) << ID << "sent " << size << " bytes";
    if (error) {
        LOG(ERROR) << ID << error.message();
        return;
    }
    if (size <= 0) {
        LOG(ERROR) << ID << "No data sent";
        return;
    }
}


bool
Connection::createFile() {
    // actual file path
    bfs::path outputpath;
    
    string destination(request.getDestination());
    if (not destination.empty()) {
        if (bfs::path(request.getFileName()).filename() == bfs::path(destination).filename())
        {
            outputpath = bfs::path(destination);
        }
        else if (destination[0] == '/')
        {
            // the destination is a absolutive path.
            outputpath = bfs::path(destination)/bfs::path(request.getFileName()).filename();
        }
        else
        {
            outputpath = basedir/bfs::path(destination)/bfs::path(request.getFileName()).filename();
        }
    } else {
        outputpath = basedir/bfs::path(request.getFileName());
    }
    LOG(INFO) << ID << "outputpath: " << outputpath;
    
    if (not bfs::exists(outputpath.parent_path())) {
        DLOG(INFO) << ID << "creating path: " << outputpath.parent_path();
        bfs::create_directories(outputpath.parent_path());
    }
    
    output.open(outputpath, ofstream::out);
    if (not output.good()) {
        LOG(ERROR) << ID << "Cannot open file: " << outputpath;
        return false;
    }
    
    DLOG(INFO) << ID << "File correctly opened";
    return true;
}


size_t
Connection::receiveFileData() {
    LOG(INFO) << ID << "receiving file data ...";
    
    size_t receivedSize = 0;
    size_t readSize = 0;
    
    do {
        boost::system::error_code error;
        readSize = socket.read_some(ba::buffer(buffer, bufferSize), error);
        if (error) {
            LOG(ERROR) << ID << error.message();
            return receivedSize;
        }
        if (readSize > 0) {
            output.write(buffer, readSize);
            receivedSize += readSize;
        }
    } while (output.tellp() != (std::streamsize) fileSize);
    
    if (output.is_open()) {
        output.close();
        DLOG(INFO) << ID << "file output closed";
    }
    
    DLOG(INFO) << ID << "received " << receivedSize << " bytes";
    return receivedSize;
}


NS_IZENELIB_DISTRIBUTE_END
