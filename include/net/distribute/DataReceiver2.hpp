/* 
 * File:   DataReceiver2.hpp
 * Author: Paolo D'Apice
 *
 * Created on May 24, 2012, 3:03 PM
 */

#ifndef DATARECEIVER2_HPP
#define	DATARECEIVER2_HPP

#include "Connection.hpp"

#include <boost/asio.hpp>
#include <boost/thread.hpp>

namespace ba = boost::asio;

NS_IZENELIB_DISTRIBUTE_BEGIN

/**
 * @brief Transfer data file server via TCP socket.
 */
class DataReceiver2 : private boost::noncopyable {
public:
    
    /**
     * Constructor.
     * @param port The port on which listen for connections.
     */
    DataReceiver2(const unsigned& port);
    
    /// Destructor.
    ~DataReceiver2();
    
    /// Start accepting incoming connections.
    void start();
    
    /// Stop accepting incoming connections.
    void stop();

private:
    
    /// Run the I/O loop.
    void run();

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
    boost::thread runThread;
};

NS_IZENELIB_DISTRIBUTE_END

#endif	/* DATARECEIVER2_HPP */
