/**
 * @file util/driver/DriverServer.cpp
 * @author Ian Yang
 * @date Created <2010-05-28 16:05:39>
 */
#include <util/driver/DriverServer.h>

#include <glog/logging.h>

#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <iostream>

namespace izenelib {
namespace driver {

DriverServer::DriverServer(
    const boost::asio::ip::tcp::endpoint& bindPort,
    const factory_ptr& connectionFactory,
    std::size_t threadPoolSize
)
: parent_type(bindPort, connectionFactory),
  threadPoolSize_(threadPoolSize)
{
    if (threadPoolSize_ < 2)
    {
        threadPoolSize_ = 2;
    }
    DriverThreadPool::init(threadPoolSize_);
}

void DriverServer::run()
{
    boost::thread_group threads;
    for (std::size_t i = 0; i < threadPoolSize_/2; ++i)
    {
        threads.create_thread(boost::bind(&DriverServer::worker, this));
    }
    threads.join_all();
    DriverThreadPool::stop();
}

void DriverServer::worker()
{
    for(;;)
    {
        try
        {
            parent_type::run();
            // normal exit
            break;
        }
        catch (std::exception& e)
        {
            DLOG(ERROR) << "[DriverServer] "
                        << e.what() << std::endl;
        }
    }
}

}
}

