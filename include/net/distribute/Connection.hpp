/* 
 * File:   Connection.hpp
 * Author: Paolo D'Apice
 *
 * Created on May 24, 2012, 3:43 PM
 */

#ifndef CONNECTION_HPP
#define	CONNECTION_HPP

#include "common.hpp"
#include "Msg.h"

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <fstream>

namespace ba = boost::asio;
namespace bfs = boost::filesystem;
namespace bs = boost::system;

NS_IZENELIB_DISTRIBUTE_BEGIN

/**
 * A file-transfer TCP connection.
 */
class Connection : public boost::enable_shared_from_this<Connection> {
public:
    
    /// Simple alias for smart pointers to \c Connection objects.
    typedef boost::shared_ptr<Connection> pointer;
    
    /**
     *  Factory method for new \c connection objects.
     * @param service a reference to the I/O service.
     * @return a \c pointer to a new \c Connection.
     */
    static pointer create(ba::io_service& service) {
        return pointer(new Connection(service));
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
    Connection(ba::io_service& service, const size_t& bufferSize = DEFAULT_BUFFER_SIZE);
    
    /// Callback for received header.
    void handleReceivedHeader(const bs::error_code&, size_t);
    
    /// Callback for sent header Ack.
    void handleSentHeaderAck(const bs::error_code&, size_t);
    
    /// Callback for received file data.
    void handleReceivedFileData(const bs::error_code&, size_t);
    
    /// Callback for sent file data Ack.
    void handleSentFileDataAck(const bs::error_code&, size_t);
    
    bool createFile(const std::string& fileName);
    
private:
    unsigned id;
    
    ba::ip::tcp::socket socket;
    
    const size_t bufferSize;
    char* buffer;
    
    std::ofstream output;
    bfs::path basedir;
    
    ::net::distribute::SendFileReqMsg header;

};

NS_IZENELIB_DISTRIBUTE_END

#endif	/* CONNECTION_HPP */
