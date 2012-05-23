/* 
 * File:   DataTransfer2.cpp
 * Author: Paolo D'Apice
 * 
 * Created on May 22, 2012, 2:54 PM
 */

#include "net/distribute/DataTransfer2.hpp"
#include "net/distribute/Msg.h"

#include <boost/lexical_cast.hpp>
#include <glog/logging.h>
#include <fstream>

using net::distribute::SendFileReqMsg;
using net::distribute::ResponseMsg;

using ba::ip::tcp;
using std::ifstream;
using std::string;


namespace izenelib {
namespace net {
namespace distribute {


DataTransfer2::DataTransfer2(const string& host_, const unsigned& port_,
            const size_t& bufferSize_)
        : host(host_), port(port_), bufferSize(bufferSize_), socket(service) {
    LOG(INFO) << "DataTransfer to " << host << ":" << port;
    buffer = new char[bufferSize_];
    DLOG(INFO) << "instantiated";
}


DataTransfer2::~DataTransfer2() {
    delete[] buffer;
    DLOG(INFO) << "destroyed";
}


bool
DataTransfer2::syncSend(const string& source, const string& destination, 
        bool recursive) {
    if (not bfs::exists(source)) {
        LOG(ERROR) << "Source: " << source << " does not exists.";
        return false;
    }
    
    if (bfs::is_directory(source)) {
        DLOG(INFO) << "directory: " << source;
        return sendDir(source, destination, recursive);
    }
    
    DLOG(INFO) << "file: " << source;
    return sendFile(source, destination);
}


void
DataTransfer2::connect() {
    DLOG(INFO) << "connecting ...";
    socket.close();
    tcp::resolver resolver(service);
    tcp::resolver::query query(host, boost::lexical_cast<string>(port));
    tcp::resolver::iterator endpoint = resolver.resolve(query);
    ba::connect(socket, endpoint);
}


bool
DataTransfer2::sendFile(const string& file, const string& destination) {
    // open the file, input mode
    ifstream input(file.c_str(), ifstream::in);
    if (not input.good()) {
        LOG(ERROR) << file << ": cannot open";
        return false;
    }
    
    // 0. connect
    try {
        connect();
    } catch (std::exception& e) {
        LOG(ERROR) << "Unable to connect: " << e.what();
        return false;
    }
    
    bfs::path filepath(file);
    size_t fileSize = bfs::file_size(filepath);
    
    bfs::path destpath(destination);
    string filename = (destpath /= filepath.relative_path()).string();
    DLOG(INFO) << file << " --> " << filename; 
    
    // 1. send head
    DLOG(INFO) << "sending request header ...";
    
    // TODO: rewrite this data structure using msgpack
    SendFileReqMsg msg;
    msg.setFileName(filename);
    msg.setFileSize(fileSize);
    string msg_head = msg.toString();
    DLOG(INFO) << "header: [" << msg_head << "]";
    
    size_t n = ba::write(socket, ba::buffer(msg_head));
    if (n != msg_head.size()) {
        LOG(ERROR) << "Sent only " << n << "/" << msg_head.size() << "bytes";
        return false;
    }
    
    // 2. receive ack
    DLOG(INFO) << "receiving header ack ...";
    
    n = socket.read_some(ba::buffer(buffer, bufferSize));
    if (n == 0) {
        LOG(ERROR) << "No data received";
        return false;
    }
    
    // TODO: rewrite this data structure using msgpack
    ResponseMsg resMsg;
    resMsg.loadMsg(string(buffer, n));
    DLOG(INFO) << "header ack: [" << resMsg.toString() << "]";
    if (resMsg.getStatus() != "success") {
        LOG(ERROR) << "Receiver not ready";
        return false;
    }

    // 3. send file data
    LOG(INFO) << "sending file: " << file << " ...";
    
    size_t sentSize = sendFileData(input);
    if (sentSize < fileSize) {
        LOG(ERROR) << "Sent " << sentSize << "/" << fileSize << " bytes";
        return false;
    }
    
    // 4. receive ack
    DLOG(INFO) << "receiving file ack ...";
    
    n = socket.read_some(ba::buffer(buffer, bufferSize));
    if (n == 0) {
        LOG(ERROR) << "No data received";
        return false;
    }
    
    resMsg.loadMsg(string(buffer, n));
    DLOG(INFO) << "file ack: [" << resMsg.toString() << "]";
    
    size_t receivedSize = resMsg.getReceivedSize();
    if (receivedSize != sentSize) {
        LOG(ERROR) << "Received " << receivedSize << "/" << sentSize << " bytes";
        return false;
    }
    
    LOG(INFO) << "file: " << file << " correctly sent";
    return true;
}


bool
DataTransfer2::sendDir(const string& dir, const string& dest, bool recursive) {
    DLOG(INFO) << "entering" << (recursive ? " (recursively) " : " ") 
               << "into: " << dir;
    
    bfs::path dirpath(dir);
    bfs::path destpath(dest);
    
    // select the proper iterator
    // TODO: can we do: if (recursive) { typedef ... iterator; } else { typedef ... iterator; } ?
    if (recursive) {
        bfs::recursive_directory_iterator end;
        for (bfs::recursive_directory_iterator it(dirpath); it != end; ++it) {
            if (not sendPath(it->path(), destpath))
                return false;
        }
    } else {
        bfs::directory_iterator end;
        for (bfs::directory_iterator it(dirpath); it != end; ++it) {
            if (not sendPath(it->path(), destpath))
                return false;
        }
    }
    
    DLOG(INFO) << "done";
    return true;
}


bool
DataTransfer2::sendPath(const bfs::path& path, const bfs::path& dest) {
    DLOG(INFO) << " * " << path.string();
    if (bfs::is_regular_file(path)) {
        return sendFile(path.string(), dest.string());
    }
    DLOG(INFO) << "skipped";
    return true;
}


size_t 
DataTransfer2::sendFileData(ifstream& input) {
    size_t sentSize = 0;
    size_t readSize = 0;
    size_t writeSize = 0;
    
    while (not input.eof()) {
        //DLOG(INFO) << "reading into buffer ...";
        input.read(buffer, bufferSize);
        readSize = input.gcount();
        //DLOG(INFO) << "read " << readSize << " bytes";
        if (readSize <= 0) { // XXX: also true for an empty file
            LOG(ERROR) << "read file error ";
            return sentSize;
        }
        
        //DLOG(INFO) << "writing " << readSize << " bytes to socket ...";
        writeSize = ba::write(socket, ba::buffer(buffer, readSize), ba::transfer_all(), error);
        if (error) {
            LOG(ERROR) << "send error:" << error;
            return sentSize;
        }
        //DLOG(INFO) << "written " << writeSize << " bytes";
        sentSize += writeSize;
    }
    
    DLOG(INFO) << "sent " << sentSize << " bytes";
    return sentSize;
}

}}}

