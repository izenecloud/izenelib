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
    normal_stop_ = false;
    parent_type::init();
    boost::thread_group threads;
    for (std::size_t i = 0; i < threadPoolSize_/2; ++i)
    {
        threads.create_thread(boost::bind(&DriverServer::worker, this));
    }
    threads.join_all();
    DriverThreadPool::stop();
}

void DriverServer::stop()
{
    normal_stop_ = true;
    LOG(INFO) << "stop driver server by request.";
    parent_type::stop();
}

void DriverServer::worker()
{
    for(;;)
    {
        try
        {
            parent_type::run();
            // normal exit
            if (normal_stop_)
                break;
            else
            {
                LOG(ERROR) << "driver stopped by accident.";
            }
        }
        catch (std::exception& e)
        {
            LOG(ERROR) << "[DriverServer] "
                << e.what() << std::endl;
        }
    }
}

}
}

