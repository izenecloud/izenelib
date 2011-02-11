//
// connections.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2009 Xu Ye Jun (moore.xu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BAS_ECHO_CLIENT_CONNECTIONS_HPP
#define BAS_ECHO_CLIENT_CONNECTIONS_HPP

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <bas/io_service_pool.hpp>
#include <bas/service_handler_pool.hpp>
#include <bas/client.hpp>

#include <iostream>

#include <ctime>

#include "client_work.hpp"
#include "client_work_allocator.hpp"

namespace echo {

class connections
{
public:
  typedef bas::service_handler_pool<client_work, client_work_allocator> client_handler_pool_type;
  typedef bas::client<client_work, client_work_allocator> client_type;

  explicit connections(const std::string& address,
      unsigned short port,
      std::size_t io_service_pool_size,
      std::size_t work_service_pool_size,
      std::size_t connection_number,
      client_handler_pool_type* service_handler_pool)
    : io_service_pool_(io_service_pool_size),
      work_service_pool_(work_service_pool_size),
      client_(address,
          port,
          service_handler_pool),
      connection_number_(connection_number)
  {
    BOOST_ASSERT(connection_number != 0);
  }

  void run()
  {
    boost::posix_time::ptime time_start = boost::posix_time::microsec_clock::universal_time();
    std::cout << "Creating " << connection_number_ << " connections.\n";

    work_service_pool_.start();
    io_service_pool_.start();

    for (std::size_t i = 0; i < connection_number_; ++i)
      client_.connect(io_service_pool_.get_io_service(), work_service_pool_.get_io_service());

    io_service_pool_.stop();
    work_service_pool_.stop();

    work_service_pool_.start();
    io_service_pool_.start();
    io_service_pool_.stop();
    work_service_pool_.stop();

    boost::posix_time::time_duration time_long = boost::posix_time::microsec_clock::universal_time() - time_start;
    std::cout << "All connections complete in " << time_long.total_milliseconds() << " ms.\n";
    std::cout.flush();

/*    boost::asio::io_service io_service;
    boost::asio::deadline_timer timer(io_service);
    timer.expires_from_now(boost::posix_time::seconds(3));
    timer.wait();
*/
  }

private:
  bas::io_service_pool io_service_pool_;

  bas::io_service_pool work_service_pool_;

  client_type client_;

  std::size_t connection_number_;
};

} // namespace echo

#endif // BAS_ECHO_CLIENT_CONNECTIONS_HPP
