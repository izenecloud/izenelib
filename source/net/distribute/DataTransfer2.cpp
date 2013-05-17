/* 
 * File:   DataTransfer2.cpp
 * Author: Paolo D'Apice
 * 
 * Created on May 22, 2012, 2:54 PM
 */

#include "net/distribute/DataTransfer2.hpp"
#include "net/distribute/Message.hpp"

#include <boost/lexical_cast.hpp>
#include <glog/logging.h>

using ba::ip::tcp;
using std::string;


NS_IZENELIB_DISTRIBUTE_BEGIN


DataTransfer2::DataTransfer2(const string& host_, const unsigned& port_,
            const size_t& bufferSize_)
        : host(host_), port(port_), bufferSize(bufferSize_), socket(service)
{
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


bool
DataTransfer2::probe() {
    try {
        connect();
        ba::write(socket, ba::buffer("probe", 5));
    } catch (std::exception& e) {
        LOG(ERROR) << "Unable to connect: " << e.what();
        return false;
    }
    return true;
}


void
DataTransfer2::connect() {
    LOG(INFO) << "connecting ...";
    socket.close();
    tcp::resolver resolver(service);
    tcp::resolver::query query(host, boost::lexical_cast<string>(port));
    tcp::resolver::iterator endpoint = resolver.resolve(query);
    ba::connect(socket, endpoint);
}

bool
DataTransfer2::sendFile(const string& file, const string& destination) {
    // open the file, input mode
    ifstream input(file.c_str());
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
    
    // 1. send head
    LOG(INFO) << "sending request header ...";
    
    bfs::path filepath(file);
    size_t fileSize = bfs::file_size(filepath);
    
    Request request(file, fileSize, destination);
    DLOG(INFO) << "request: " << request;
    
    // serialize and send
    msgpack::sbuffer sbuff;
    msgpack::pack(sbuff, request);
    size_t n = ba::write(socket, ba::buffer(sbuff.data(), sbuff.size()), error);
    if (error) {
        LOG(ERROR) << error.message();
        return false;
    }
    if (n != sbuff.size()) {
        LOG(ERROR) << "Sent only " << n << "/" << sbuff.size() << "bytes";
        return false;
    }
    
    // 2. receive ack
    LOG(INFO) << "receiving header ack ...";
    
    n = socket.read_some(ba::buffer(buffer, bufferSize), error);
    if (error) {
        LOG(ERROR) << error.message();
        return false;
    }
    if (n == 0) {
        LOG(ERROR) << "No data received";
        return false;
    }
    
    // deserialize
    msgpack::unpacked unp;
    msgpack::unpack(&unp, buffer, n);
    RequestAck ack = unp.get().as<RequestAck>();
    DLOG(INFO) << "request ack: " << ack;
    if (not ack.getStatus()) {
        LOG(ERROR) << "Receiver error";
        return false;
    }

    // 3. send file data
    LOG(INFO) << "sending file: " << file << " ...";
    
    size_t sentSize = sendFileData(input);
    if (sentSize != fileSize) {
        LOG(ERROR) << "Sent " << sentSize << "/" << fileSize << " bytes";
        return false;
    }
    
    // 4. receive ack
    DLOG(INFO) << "receiving file ack ...";
    
    n = socket.read_some(ba::buffer(buffer, bufferSize), error);
    if (error) {
        LOG(ERROR) << error.message();
        return false;
    }
    if (n == 0) {
        LOG(ERROR) << "No data received";
        return false;
    }
    
    // deserialize
    msgpack::unpack(&unp, buffer, n);
    ack = unp.get().as<RequestAck>();
    DLOG(INFO) << "file ack: " << ack;
    if (not ack.getStatus()) {
        LOG(ERROR) << "Receiver error";
        return false;
    }
    size_t receivedSize = ack.getSize();
    if (receivedSize != sentSize) {
        LOG(ERROR) << "Received " << receivedSize << "/" << sentSize << " bytes";
        return false;
    }
    
    LOG(INFO) << "file: " << file << " correctly sent";
    return true;
}


bool
DataTransfer2::sendDir(const bfs::path& dirpath, const bfs::path& destpath, bool recursive) {
    DLOG(INFO) << "entering" << (recursive ? " (recursively) " : " ") 
               << "into: " << dirpath;
    
    // select the proper iterator
    // XXX: cannot do: if (recursive) { typedef ... iterator; } else { typedef ... iterator; } ?
    static const bfs::directory_iterator end;
    for (bfs::directory_iterator it(dirpath); it != end; ++it) {

        if (bfs::is_directory(it->path()))
        {
            if (recursive && !sendDir(it->path(), destpath/it->path().filename(), recursive))
                return false;
        }
        else if (bfs::is_regular_file(it->path()))
        {
            bfs::path send_dest = destpath;
            if (it->path() == destpath/it->path().filename())
                send_dest = it->path();

            if (not sendFile(it->path().string(), send_dest.string()))
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
            LOG(ERROR) << "send error:" << error.message();
            return sentSize;
        }
        //DLOG(INFO) << "written " << writeSize << " bytes";
        sentSize += writeSize;
    }
    
    DLOG(INFO) << "sent " << sentSize << " bytes";
    return sentSize;
}


NS_IZENELIB_DISTRIBUTE_END
