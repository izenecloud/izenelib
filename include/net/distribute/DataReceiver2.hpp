/* 
 * File:   DataReceiver2.hpp
 * Author: Paolo D'Apice
 *
 * Created on May 24, 2012, 3:03 PM
 */

#ifndef IZENELIB_NET_DISTRIBUTE_DATARECEIVER2_HPP
#define	IZENELIB_NET_DISTRIBUTE_DATARECEIVER2_HPP

#include "Connection.hpp"

#include <boost/asio.hpp>
#include <boost/thread.hpp>

namespace ba = boost::asio;

NS_IZENELIB_DISTRIBUTE_BEGIN

/**
 * @brief File transfer server.
 * Transfer data file server via TCP socket.
 */
class DataReceiver2 : private boost::noncopyable {
public:
    
    /**
     * Constructor.
     * @param basedir The directory for saved files.
     * @param port The port on which listen for connections.
     *          Default: \c DEFAULT_PORT
     * @param bufferSize The buffer size. 
     *          Default: \c DEFAULT_BUFFER_SIZE
     * @param handleSignals Install signals handlers.
     *          Default: \c false
     */
    DataReceiver2(const std::string& basedir,
            const unsigned& port = DEFAULT_PORT,
            const size_t& bufferSize = DEFAULT_BUFFER_SIZE,
            bool handleSignals = false);
    
    /// Destructor.
    ~DataReceiver2();
    
    /// Start accepting incoming connections.
    void start();
    
    /// Stop accepting incoming connections.
    void stop();

private:
    
    /// Start the service.
    void startAccept();
    
    /// Callback for incoming connection.
    void handleAccept(Connection::pointer connection,
                const boost::system::error_code& error);
    
    /// Callback for stop signals.
    void handleStop();
    
private:
    
    ba::io_service service;
    ba::ip::tcp::endpoint endpoint;
    ba::ip::tcp::acceptor acceptor;
    
    ba::signal_set signals;
    
    const std::string basedir;
    const size_t bufferSize;

    bool running;
};

NS_IZENELIB_DISTRIBUTE_END

#endif	/* IZENELIB_NET_DISTRIBUTE_DATARECEIVER2_HPP */
