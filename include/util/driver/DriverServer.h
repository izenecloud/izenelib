#ifndef IZENELIB_DRIVER_SERVER_H
#define IZENELIB_DRIVER_SERVER_H
/**
 * @file util/driver/DriverServer.h
 * @author Ian Yang
 * @date Created <2010-05-26 16:15:30>
 * @brief Driver Server
 */
#include "DriverConnection.h"

#include <net/tcpserver/AsyncServer.h>

namespace izenelib {
namespace driver {

class DriverServer
: public tcpserver::AsyncServer<DriverConnectionFactory>
{
public:
    typedef tcpserver::AsyncServer<DriverConnectionFactory> parent_type;
    typedef boost::shared_ptr<DriverConnectionFactory> factory_ptr;

    DriverServer(
        const boost::asio::ip::tcp::endpoint& bindPort,
        const factory_ptr& connectionFactory,
        std::size_t threadPoolSize
    );

    void run();
    void stop();

private:
    void worker();

    std::size_t threadPoolSize_;
    bool normal_stop_;
};

}
}

#endif // IZENELIB_DRIVER_SERVER_H
