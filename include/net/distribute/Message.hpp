/* 
 * File:   Message.hpp
 * Author: Paolo D'Apice
 *
 * Created on May 28, 2012, 2:03 PM
 */

#ifndef IZENELIB_NET_DISTRIBUTE_MESSAGE_HPP
#define	IZENELIB_NET_DISTRIBUTE_MESSAGE_HPP

#include "common.hpp"
#include <3rdparty/msgpack/msgpack.hpp>
#include <boost/noncopyable.hpp>

NS_IZENELIB_DISTRIBUTE_BEGIN

/**
 * @brief File transfer request message.
 */
class Request {
public:
    
    /**
     * Creates a new request message.
     * @param name The file name.
     * @param size The file size.
     * @param destination The destination.
     */
    Request(const std::string& name, const size_t& size, const std::string& dest) 
        : fileName(name), fileSize(size), destination(dest) {}
    
    /// Destructor.
    ~Request() {}
    
    /// @return The file name.
    std::string getFileName() const {
        return fileName;
    }
    
    /// @return The file size.
    size_t getFileSize() const {
        return fileSize;
    }
    
    /// @return The destination.
    std::string getDestination() const {
        return destination;
    }
    
    friend std::ostream& operator<<(std::ostream& os, const Request& r) {
        return os << "[fileName="   << r.fileName    << ","
                  << "fileSize="    << r.fileSize    << ","
                  << "destination=" << r.destination << "]";
    }
    
private:
    std::string fileName;
    size_t fileSize;
    std::string destination;
    
public: // msgpack specific
    Request() {}
    MSGPACK_DEFINE(fileName, fileSize, destination);
};


/**
 * @brief File transfer acknowledgment message.
 */
class RequestAck {
public:
    
    /**
     * Creates a new ack message.
     * @param st The status.
     * @param sz (optional) The size.
     */
    RequestAck(const bool& st, const size_t& sz = 0) : status(st), size(sz) {}
    
    /// Destructor.
    ~RequestAck() {}
    
    /// @return  The status.
    bool getStatus() const {
        return status;
    }
    
    /// @return The size.
    size_t getSize() const {
        return size;
    }
    
    friend std::ostream& operator<<(std::ostream& os, const RequestAck& r) {
        return os << "[status=" << r.status    << ","
                  << "size="    << r.size      << "]";
    }
    
private:
    bool status;
    size_t size;
    
public: // msgpack specific
    RequestAck() {}
    MSGPACK_DEFINE(status, size);
};

NS_IZENELIB_DISTRIBUTE_END

#endif	/* IZENELIB_NET_DISTRIBUTE_MESSAGE_HPP */
