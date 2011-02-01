//
// client_work.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2009 Xu Ye Jun (moore.xu@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BAS_ECHO_CLIENT_WORK_HPP
#define BAS_ECHO_CLIENT_WORK_HPP

#include <bas/service_handler.hpp>

#include <iostream>

namespace echo {

std::string echo_message = "message from client";

class client_work
{
public:
    typedef bas::service_handler<client_work> client_handler_type;

    client_work()
    {
    }

    void on_clear(client_handler_type& handler)
    {
    }

    void on_open(client_handler_type& handler)
    {
        handler.async_write(boost::asio::buffer(echo_message, echo_message.size()));
    }

    void on_read(client_handler_type& handler, std::size_t bytes_transferred)
    {
        std::cout << "receive: " << handler.read_buffer().data() << " " << bytes_transferred << std::endl;
        handler.read_buffer().clear();
        handler.close();
    }

    void on_write(client_handler_type& handler, std::size_t bytes_transferred)
    {
        handler.async_read_some();
    }

    void on_close(client_handler_type& handler, const boost::system::error_code& e)
    {
        switch (e.value())
        {
            // Operation successfully completed.
            case 0:
            case boost::asio::error::eof:
            break;

            // Operation timed out.
            case boost::asio::error::timed_out:
            std::cout << "client error " << e << " message " << e.message() << "\n";
            std::cout.flush();
            break;

            // Connection breaked.
            case boost::asio::error::connection_aborted:
            case boost::asio::error::connection_reset:
            case boost::asio::error::connection_refused:
            break;

            // Other error.
            case boost::asio::error::no_buffer_space:
            std::cout << "client error " << e << " message " << e.message() << "\n";
            std::cout.flush();
            break;
        }
    }

    void on_parent(client_handler_type& handler, const bas::event event)
    {
    }

    void on_child(client_handler_type& handler, const bas::event event)
    {
    }
};

} // namespace echo

#endif // BAS_ECHO_CLIENT_WORK_HPP
