//
// service_handler_pool.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2009 Xu Ye Jun (moore.xu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BAS_SERVICE_HANDLER_POOL_HPP
#define BAS_SERVICE_HANDLER_POOL_HPP

#include <boost/assert.hpp>
#include <boost/asio/detail/mutex.hpp>
#include <boost/noncopyable.hpp>
#include <vector>

#include <bas/service_handler.hpp>

namespace bas {

#define BAS_GRACEFUL_CLOSED_WAIT_DELAY 5

/// A pool of service_handler objects.
template<typename Work_Handler, typename Work_Allocator, typename Socket_Service = boost::asio::ip::tcp::socket>
class service_handler_pool
  : private boost::noncopyable
{
public:
  /// The type of the service_handler.
  typedef service_handler<Work_Handler, Socket_Service> service_handler_type;
  typedef boost::shared_ptr<service_handler_type> service_handler_ptr;

  /// The type of the work_allocator.
  typedef Work_Allocator work_allocator_type;
  typedef boost::shared_ptr<work_allocator_type> work_allocator_ptr;

  /// Construct the service_handler pool.
  explicit service_handler_pool(work_allocator_type* work_allocator,
      std::size_t initial_pool_size,
      std::size_t read_buffer_size,
      std::size_t write_buffer_size = 0,
      std::size_t timeout_seconds = 0,
      std::size_t closed_wait_delay = BAS_GRACEFUL_CLOSED_WAIT_DELAY)
    : service_handlers_(),
      work_allocator_(work_allocator),
      initial_pool_size_(initial_pool_size),
      read_buffer_size_(read_buffer_size),
      write_buffer_size_(write_buffer_size),
      timeout_seconds_(timeout_seconds),
      closed_wait_delay_(closed_wait_delay),
      next_service_handler_(0)
  {
    BOOST_ASSERT(work_allocator != 0);
    BOOST_ASSERT(initial_pool_size != 0);

    // Create preallocated service_handler pool.
    for (std::size_t i = 0; i < initial_pool_size_; ++i)
    {
      service_handler_ptr service_handler(make_handler());
      service_handlers_.push_back(service_handler);
    }
  }

  /// Destruct the pool object.
  ~service_handler_pool()
  {
    for (std::size_t i = 0; i < service_handlers_.size(); ++i)
      service_handlers_[i].reset();
    service_handlers_.clear();

    work_allocator_.reset();
  }

  /// Get an service_handler to use.
  service_handler_ptr get_service_handler(boost::asio::io_service& io_service,
      boost::asio::io_service& work_service)
  {
    service_handler_ptr service_handler;

    // Check the next handler is busy or not.
    if (!service_handlers_[next_service_handler_]->is_busy())
    {
      service_handler = service_handlers_[next_service_handler_];
      if (++next_service_handler_ == service_handlers_.size())
        next_service_handler_ = 0;
    }
    else
      next_service_handler_ = 0;

    // If the next handler is busy, create new handler.
    if (service_handler.get() == 0)
    {
      service_handler.reset(make_handler());
      service_handlers_.push_back(service_handler);
    }

    // Bind the service handler with given io_service and work_service.
    service_handler->bind(io_service, work_service, work_allocator());
    return service_handler;
  }

  /// Get an service_handler with the given mutex.
  service_handler_ptr get_service_handler(boost::asio::io_service& io_service,
      boost::asio::io_service& work_service,
      boost::asio::detail::mutex& mutex)
  {
    // For client handler, need lock in multiple thread model.
    boost::asio::detail::mutex::scoped_lock lock(mutex);
    return get_service_handler(io_service, work_service);
  }

private:

  /// Get the allocator for work.
  work_allocator_type& work_allocator()
  {
    return *work_allocator_;
  }

  /// Make one service_handler.
  service_handler_type* make_handler()
  {
    return new service_handler_type(work_allocator().make_handler(),
        read_buffer_size_,
        write_buffer_size_,
        timeout_seconds_,
        closed_wait_delay_);
  }

private:

  /// The pool of preallocated service_handler.
  std::vector<service_handler_ptr> service_handlers_;

  /// The allocator of work_handler.
  work_allocator_ptr work_allocator_;

  /// Preallocated service_handler number.
  std::size_t initial_pool_size_;

  /// The maximum size for asynchronous read operation buffer.
  std::size_t read_buffer_size_;

  /// The maximum size for asynchronous write operation buffer.
  std::size_t write_buffer_size_;

  /// The expiry seconds of connection.
  std::size_t timeout_seconds_;

  /// The delay seconds before reuse again.
  std::size_t closed_wait_delay_;

  /// The next service_handler to use for a connection.
  std::size_t next_service_handler_;
};

} // namespace bas

#endif // BAS_SERVICE_HANDLER_POOL_HPP
