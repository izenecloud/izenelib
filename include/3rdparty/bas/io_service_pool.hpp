//
// io_service_pool.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2009 Xu Ye Jun (moore.xu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BAS_IO_SERVICE_POOL_HPP
#define BAS_IO_SERVICE_POOL_HPP

#include <boost/assert.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace bas {

/// A pool of io_service objects.
class io_service_pool
  : private boost::noncopyable
{
public:
  /// Construct the io_service pool.
  explicit io_service_pool(std::size_t pool_size)
    : io_services_(),
      work_(),
      threads_(),
      next_io_service_(0),
      block_(false)
  {
    BOOST_ASSERT(pool_size != 0);

    // Create io_service pool.
    for (std::size_t i = 0; i < pool_size; ++i)
    {
      io_service_ptr io_service(new boost::asio::io_service);
      io_services_.push_back(io_service);
    }
  }
  
  /// Destruct the pool object.
  ~io_service_pool()
  {
    stop();

    // Destroy all work.
    for (std::size_t i = 0; i < work_.size(); ++i)
      work_[i].reset();
    work_.clear();

    // Destroy io_service pool.
    for (std::size_t i = 0; i < io_services_.size(); ++i)
      io_services_[i].reset();
    io_services_.clear();
  }

  std::size_t size()
  {
    return io_services_.size();
  }

  /// Start all io_service objects in nonblock model.
  void start()
  {
    start(false);
  }

  /// Run all io_service objects in block model.
  void run()
  {
    start(true);
  }

  /// Stop all io_service objects in the pool.
  void stop()
  {
    if (threads_.size() == 0)
      return;

    // Allow all operations and handlers to be finished normally,
    // the work object may be explicitly destroyed.

    // Destroy all work.
    for (std::size_t i = 0; i < work_.size(); ++i)
      work_[i].reset();
    work_.clear();

    if (!block_)
      wait();
  }

  /// Get an io_service to use.
  boost::asio::io_service& get_io_service()
  {
    boost::asio::io_service& io_service = *io_services_[next_io_service_];
    if (++next_io_service_ == io_services_.size())
      next_io_service_ = 0;
    return io_service;
  }

  /// Get the certain io_service to use.
  boost::asio::io_service& get_io_service(std::size_t offset)
  {
    if (offset < io_services_.size())
      return *io_services_[offset];
    else
      return get_io_service();
  }

private:
  /// Wait for all threads in the pool to exit.
  void wait()
  {
    if (threads_.size() == 0)
      return;

    // Wait for all threads in the pool to exit.
    for (std::size_t i = 0; i < threads_.size(); ++i)
      threads_[i]->join();

    // Destroy all threads.
    threads_.clear();
  }

  /// Start all io_service objects in the pool.
  void start(bool block)
  {
    if (threads_.size() != 0)
      return;

    // Reset the io_service in preparation for a subsequent run() invocation.
    for (std::size_t i = 0; i < io_services_.size(); ++i)
    {
      io_services_[i]->reset();
    }

    for (std::size_t i = 0; i < io_services_.size(); ++i)
    {
      // Give the io_service work to do so that its run() functions will not
      // exit until work was explicitly destroyed.
      work_ptr work(new boost::asio::io_service::work(*io_services_[i]));
      work_.push_back(work);

      // Create a thread to run the io_service.
      thread_ptr thread(new boost::thread(
          boost::bind(&boost::asio::io_service::run, io_services_[i])));
      threads_.push_back(thread);
    }
  
    block_ = block;
  
    if (block)
      wait();
  }

private:
  typedef boost::shared_ptr<boost::asio::io_service> io_service_ptr;
  typedef boost::shared_ptr<boost::asio::io_service::work> work_ptr;
  typedef boost::shared_ptr<boost::thread> thread_ptr;

  /// The pool of io_services.
  std::vector<io_service_ptr> io_services_;

  /// The work that keeps the io_services running.
  std::vector<work_ptr> work_;

  /// The pool of threads for running individual io_service.
  std::vector<thread_ptr> threads_;

  /// The next io_service to use for a connection.
  std::size_t next_io_service_;

  /// Flag to indicate that start() functions will block or not.
  bool block_;
};

} // namespace bas

#endif // BAS_IO_SERVICE_POOL_HPP
