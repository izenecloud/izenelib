/* 
 * File:   Connection.hpp
 * Author: Paolo D'Apice
 *
 * Created on May 24, 2012, 3:43 PM
 */

#ifndef IZENELIB_NET_DISTRIBUTE_CONNECTION_HPP
#define	IZENELIB_NET_DISTRIBUTE_CONNECTION_HPP

#include "common.hpp"
#include "Message.hpp"

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/shared_ptr.hpp>

namespace ba = boost::asio;
namespace bfs = boost::filesystem;
namespace bs = boost::system;

NS_IZENELIB_DISTRIBUTE_BEGIN

/**
 * @brief A file-transfer TCP connection.
 */
class Connection : public boost::enable_shared_from_this<Connection> {
public:
    
    /// Simple alias for smart pointers to \c Connection objects.
    typedef boost::shared_ptr<Connection> pointer;
    
    /**
     * Factory method for new \c connection objects.
     * @param service A reference to the I/O service.
     * @param bufferSize The buffer size. 
     * @param dir The directory for saved files.
     * @return a \c pointer to a new \c Connection.
     */
    static pointer create(ba::io_service& service, const size_t& bufferSize,
            const std::string& dir) {
        return pointer(new Connection(service, bufferSize, dir));
    }
    
    /// @return a reference to the socket.
    ba::ip::tcp::socket& getSocket() {
        return socket;
    }
    
    /// Start data transfer on this connection.
    void start();
    
    /// Destructor.
    ~Connection();
    
private:
    
    /// Private constructor.
    Connection(ba::io_service& service, const size_t& bufferSize, 
            const std::string& dir);
    
    /// Callback for received header.
    void handleReceivedHeader(const bs::error_code&, size_t);
    
    /// Callback for sent header Ack.
    void handleSentHeaderAck(const bs::error_code&, size_t);
    
    /// Callback for sent file data Ack.
    void handleSentFileDataAck(const bs::error_code&, size_t);
    
    bool createFile();
    
    size_t receiveFileData();
    
private:
    
    const unsigned id;
    ba::ip::tcp::socket socket;
    
    const size_t bufferSize;
    char* buffer;
    
    bfs::ofstream output;
    bfs::path basedir;
    size_t fileSize;
    
    Request request;
};

NS_IZENELIB_DISTRIBUTE_END

#endif	/* CONNECTION_HPP */
