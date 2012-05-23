/* 
 * File:   DataTransfer2.hpp
 * Author: Paolo D'Apice
 *
 * Created on May 22, 2012, 2:54 PM
 */

#ifndef DATATRANSFER2_HPP
#define	DATATRANSFER2_HPP

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

namespace ba = boost::asio;
namespace bfs = boost::filesystem;

namespace izenelib {
namespace net {
namespace distribute {

namespace {
/// Default buffer size. Value: 64 KiB.
const size_t DEFAULT_BUFFER_SIZE = 64 * 1024;
/// Default port. Value: 18121.
const unsigned DEFAULT_PORT = 18121;
/// Default destination dir. Value: ".".
const std::string DEFAULT_DESTINATION = ".";
}

/**
 * @brief Transfer data files via TCP socket.
 */
class DataTransfer2 : private boost::noncopyable {
public:
    
    /**
     * Constructor.
     * @param host The destination address.
     * @param port The destination port.
     *          Default: \c DEFAULT_PORT
     * @param bufferSize the buffer size. 
     *          Default: \c DEFAULT_BUFFER_SIZE
     */
    DataTransfer2(const std::string& host, const unsigned& port = DEFAULT_PORT, 
            const size_t& bufferSize = DEFAULT_BUFFER_SIZE);
    
    /// Destructor.
    ~DataTransfer2();

    /**
     * Synchronously send file or directory.
     * @param source 
     *          Path to the source file or directory be sent.
     * @param destination
     *          Target directory in remote host relative to its basedir.
     *          Default: \c DEFAULT_DESTINATION
     * @param recursive  
     *          Flag indicating whether send file recursively (only available
     *          when \c source is a directory).
     *          Default: false
     * @return \c true on success, \c false on failure.
     */
     bool syncSend(const std::string& source, 
                const std::string& destination = DEFAULT_DESTINATION, 
                bool recursive = false);
     
private:
    
    /// Connect to the destination.
    void connect();
    
    /// Send a file.
    bool sendFile(const std::string& file, const std::string& dest);
    
    /// Send a directory.
    bool sendDir(const std::string& dir, const std::string& dest, bool recursive);
    
    /// Actually send files in directory.
    bool sendPath(const bfs::path& path, const bfs::path& dest);
    
    /// Actually send file data.
    size_t sendFileData(std::ifstream& input);
    
private:
    
    const std::string host;
    const unsigned port;
    
    const size_t bufferSize;
    char* buffer;
    
    ba::io_service service;
    ba::ip::tcp::socket socket;
    boost::system::error_code error;
};

}}} /* izenelib::net::distribute */

#endif	/* DATATRANSFER2_HPP */
