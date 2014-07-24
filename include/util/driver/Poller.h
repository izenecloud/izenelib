#ifndef IZENELIB_DRIVER_POLLER_H
#define IZENELIB_DRIVER_POLLER_H
/**
 * @file izenelib/driver/Poller.h
 * @author Ian Yang
 * @date Created <2010-06-18 19:01:39>
 */

#include <boost/asio.hpp>

namespace izenelib {
namespace driver {

class Poller
{
public:
    explicit Poller(boost::asio::ip::tcp::socket& socket)
    : socket_(&socket)
    {}

    Poller()
    : socket_(0)
    {}

    template<typename Handler>
    void poll(Handler handler)
    {
        if (socket_)
        {
            // schedule the request
            // Use null_buffers() tricks to bind the event with the socket instead
            // of using post(). So when socket is shutdown, all events are
            // canceled.

            boost::asio::async_write(
                *socket_,
                boost::asio::null_buffers(),
                handler
            );
        }
    }

    bool valid() const
    {
        return socket_;
    }

    boost::asio::io_service& get_io_service()
    {
        return socket_->get_io_service();
    }

private:
    boost::asio::ip::tcp::socket* socket_;
};

}} // namespace izenelib::driver

#endif // IZENELIB_DRIVER_POLLER_H
