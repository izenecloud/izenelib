//
// server.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2009 Xu Ye Jun (moore.xu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BAS_SERVER_HPP
#define BAS_SERVER_HPP

#include <boost/assert.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <bas/io_service_pool.hpp>
#include <bas/service_handler.hpp>
#include <bas/service_handler_pool.hpp>

namespace bas {

/// The top-level class of the server.
template<typename Work_Handler, typename Work_Allocator, typename Socket_Service = boost::asio::ip::tcp::socket>
class server
  : private boost::noncopyable
{
public:
  /// The type of the service_handler.
  typedef service_handler<Work_Handler, Socket_Service> service_handler_type;
  typedef boost::shared_ptr<service_handler_type> service_handler_ptr;

  /// The type of the service_handler_pool.
  typedef service_handler_pool<Work_Handler, Work_Allocator, Socket_Service> service_handler_pool_type;
  typedef boost::shared_ptr<service_handler_pool_type> service_handler_pool_ptr;

  /// Construct the server to listen on the specified TCP address and port.
  explicit server(const std::string& address,
      unsigned short port,
      std::size_t io_service_pool_size,
      std::size_t work_service_pool_size,
      service_handler_pool_type* service_handler_pool)
    : accept_service_pool_(1),
      io_service_pool_(io_service_pool_size),
      work_service_pool_(work_service_pool_size),
      service_handler_pool_(service_handler_pool),
      acceptor_(accept_service_pool_.get_io_service()),
      endpoint_(boost::asio::ip::address::from_string(address), port),
      started_(false)
  {
    BOOST_ASSERT(service_handler_pool != 0);
  }
  
  /// Destruct the server object.
  ~server()
  {
    // Stop the server's io_service loop.
    stop();
    // Destroy service_handler pool.
    service_handler_pool_.reset();
  }

  /// Run the server's io_service loop.
  void run()
  {
    if (started_)
      return;

    // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
    acceptor_.open(endpoint_.protocol());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint_);
    acceptor_.listen();
      // Accept new connection.
    accept_one();

    // Start work_service_pool with nonblock to perform synchronous works.
    work_service_pool_.start();
    // Start io_service_pool with nonblock to perform asynchronous i/o operations.
    io_service_pool_.start();

    started_ = true;
    // Start accept_service_pool with block to perform asynchronous accept operations.
    accept_service_pool_.run();

    // Stop io_service_pool.
    io_service_pool_.stop();
    // Stop work_service_pool.
    work_service_pool_.stop();

    // Dispatch ready handlers.
    for (std::size_t i = 0; i < 5; ++i)
    {
      work_service_pool_.start();
      io_service_pool_.start();
      io_service_pool_.stop();
      work_service_pool_.stop();
    }

    started_ = false;
  }

  /// Stop the server.
  void stop()
  {
    if (!started_)
      return;

    // Close the acceptor in the same thread.
    acceptor_.get_io_service().dispatch(boost::bind(&boost::asio::ip::tcp::acceptor::close,
        &acceptor_));

    // Stop accept_service_pool from block.
    accept_service_pool_.stop();
  }

private:
  /// Start to accept one connection.
  void accept_one()
  {
    // Get new handler for accept, and bind with acceptor's io_service.
    service_handler_ptr handler = service_handler_pool_->get_service_handler(io_service_pool_.get_io_service(),
        work_service_pool_.get_io_service());
    // Use new handler to accept.
    acceptor_.async_accept(handler->socket().lowest_layer(),
        boost::bind(&server::handle_accept,
            this,
            boost::asio::placeholders::error,
            handler));
  }

  /// Handle completion of an asynchronous accept operation.
  void handle_accept(const boost::system::error_code& e,
      service_handler_ptr handler)
  {
    if (!e)
    {
      // Start the first operation of the current handler.
      handler->start();
      // Accept new connection.
      accept_one();
    }
    else
      handler->close(e);
  }

private:
  /// The pool of io_service objects used to perform asynchronous accept operations.
  io_service_pool accept_service_pool_;

  /// The pool of io_service objects used to perform asynchronous i/o operations.
  io_service_pool io_service_pool_;

  /// The pool of io_service objects used to perform synchronous works.
  io_service_pool work_service_pool_;

  /// The pool of service_handler objects.
  service_handler_pool_ptr service_handler_pool_;

  /// The acceptor used to listen for incoming connections.
  boost::asio::ip::tcp::acceptor acceptor_;

  /// The server endpoint.
  boost::asio::ip::tcp::endpoint endpoint_;

  // Flag to indicate that the server is started or not.
  bool started_;
};

} // namespace bas

#endif // BAS_SERVER_HPP
