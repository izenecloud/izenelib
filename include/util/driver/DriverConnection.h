#ifndef IZENELIB_DRIVER_CONNECTION_H
#define IZENELIB_DRIVER_CONNECTION_H
/**
 * @file util/driver/DriverConnection.h
 * @author Ian Yang
 * @date Created <2010-05-26 14:06:08>
 * @brief Handle a single connection from client.
 */
#include "DriverConnectionContext.h"

#include "Router.h"
#include "Reader.h"
#include "Writer.h"
#include "Poller.h"

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/threadpool.hpp>

namespace izenelib {
namespace driver {

class DriverConnection
: public boost::enable_shared_from_this<DriverConnection>,
  private boost::noncopyable
{
    typedef boost::shared_ptr<DriverConnection> connection_ptr;
    typedef boost::shared_ptr<DriverConnectionContext> context_ptr;
    typedef boost::shared_ptr<driver::Router> router_ptr;

    typedef boost::asio::ip::address ip_address;

public:
    typedef boost::function1<bool, const ip_address&> firewall_type;

    DriverConnection(
        boost::asio::io_service& ioService,
        const router_ptr& router,
        firewall_type firewall = firewall_type()
    );

    /// @brief Socket used to communicate with client.
    inline boost::asio::ip::tcp::socket& socket()
    {
        return socket_;
    }

    /// @brief Start to handle request received from the connection.
    void start();

private:
    /// @brief Read next form size and sequence asynchronously.
    void asyncReadFormHeader();

    /// @brief Handler called after form header has been received.
    void afterReadFormHeader(const boost::system::error_code& e);

    /// @brief Read request header and body of next request form.
    void asyncReadFormPayload();

    /// @brief Handler called after request header and body has been received.
    void afterReadFormPayload(const boost::system::error_code& e);

    /// @brief Handle the request
    void handleRequest(context_ptr context,
                       const boost::system::error_code& e);
    void handleRequestFunc(context_ptr context);

    /// @brief Write response asynchronously.
    void asyncWriteResponse(context_ptr context);

    void asyncWriteError(const context_ptr& context,
                         const std::string& message);

    /// @brief Handler called after write.
    ///
    /// It keep a reference to this object and context.
    void afterWriteResponse(context_ptr);

    /// @brief Shutdown receive end.
    void shutdownReceive();

    /// @brief Create a new context from the buffer \c nextContext_
    context_ptr createContext();

    /// @brief Called on read error
    void onReadError(const boost::system::error_code& e);

    /// @brief Socket to communicate with the client in this connection.
    boost::asio::ip::tcp::socket socket_;

    driver::Poller poller_;

    /// @brief Router used to look up associated action handler.
    router_ptr router_;

    /// @brief Buffer to read size and request body
    DriverConnectionContext nextContext_;

    /// @brief Firewall. Only address that gets return value true can send
    /// request.
    firewall_type firewall_;

    /// @brief Request reader
    boost::scoped_ptr<driver::Reader> reader_;

    /// @brief Response writer
    boost::scoped_ptr<driver::Writer> writer_;

    // @brief size limit
    enum {
        kLimitSize = 64 * 1024 * 1024 // 64M
    };
};

class DriverThreadPool
{
public:
    typedef boost::shared_ptr<boost::threadpool::pool> threadpool_ptr;
    static void init(size_t poolsize);
    static void stop();
    static void schedule_task(const boost::threadpool::pool::task_type& task);

private:
    static threadpool_ptr driver_pool_;
};

class DriverConnectionFactory
{
public:
    typedef boost::shared_ptr<driver::Router> router_ptr;
    typedef DriverConnection::firewall_type firewall_type;

    DriverConnectionFactory(const router_ptr& router);

    typedef DriverConnection connection_type;
    inline DriverConnection* create(boost::asio::io_service& ioService) const
    {
        return new DriverConnection(ioService, router_, firewall_);
    }

    void setFirewall(firewall_type firewall);

private:
    router_ptr router_;
    firewall_type firewall_;
};

}
}

#endif // IZENELIB_DRIVER_CONNECTION_H
