#ifndef NET_TCPSERVER_ASYNC_SERVER_H
#define NET_TCPSERVER_ASYNC_SERVER_H
/**
 * @file net/tcpserver/AsyncServer.h
 * @author Ian Yang
 * @date Created <2010-05-26 15:23:00>
 * @brief Asynchronous TCP server
 */

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

namespace tcpserver {

template<typename ConnectionFactory>
class AsyncServer : private boost::noncopyable
{
public:
    typedef typename ConnectionFactory::connection_type connection_type;
    typedef boost::shared_ptr<connection_type> connection_ptr;

    typedef boost::function1<void, const boost::system::error_code&> error_handler;

    /**
     * @brief Construct a server listening on the specified TCP address and
     * port.
     * @param address listen address
     * @param port listen port
     */
    explicit AsyncServer(
        const boost::asio::ip::tcp::endpoint& bindPort,
        const boost::shared_ptr<ConnectionFactory>& connectionFactory
    );

    ~AsyncServer(){}

    void init();
    /// @brief Run main loop.
    /// To handle connection in threads pool, please run this method in all
    /// threads.
    ///
    /// e.g.
    ///
    /// <code>
    /// AsyncServer<Factory> server(address, port, factory);
    /// boost::thread_group threads;
    /// std::size_t poolSize = 10;
    /// for (std::size_t i = 0; i < poolSize; ++i) {
    ///     threads.create_thread(boost::bind(&AsyncServer<Factory>::run, &server));
    /// }
    /// threads.join_all();
    /// </code>
    inline void run();
    inline void reset();

    /// @brief Stop the server.
    inline void stop();

    /// @brief Set a handler to process error while listening.
    inline void setErrorHandler(error_handler handler);

private:
    inline void listen(const boost::asio::ip::tcp::endpoint& bindPort);

    inline void asyncAccept();

    /// @brief Accept a incoming connection asynchronously.
    void onAccept(const boost::system::error_code& e);

    /// @brief Handle error
    inline void onError(const boost::system::error_code& e);

    /// @brief The io_service used to perform asynchronous operations.
    boost::asio::io_service ioService_;
    boost::asio::ip::tcp::endpoint bindPort_;

    /// @brief Acceptor used to listen for incoming connections.
    boost::asio::ip::tcp::acceptor acceptor_;

    /// @brief Factory used to create new connection handler
    boost::shared_ptr<ConnectionFactory> connectionFactory_;

    /// @brief The next connection to be accepted.
    connection_ptr newConnection_;

    error_handler errorHandler_;
};

template<typename ConnectionFactory>
AsyncServer<ConnectionFactory>::AsyncServer(
    const boost::asio::ip::tcp::endpoint& bindPort,
    const boost::shared_ptr<ConnectionFactory>& connectionFactory
)
: ioService_()
, bindPort_(bindPort)
, acceptor_(ioService_)
, connectionFactory_(connectionFactory)
, newConnection_()
, errorHandler_()
{
}

template<typename ConnectionFactory>
void AsyncServer<ConnectionFactory>::init(
    )
{
    listen(bindPort_);
    asyncAccept();
}

template<typename ConnectionFactory>
void AsyncServer<ConnectionFactory>::run()
{
    ioService_.run();
}

template<typename ConnectionFactory>
void AsyncServer<ConnectionFactory>::reset()
{
    ioService_.reset();
}

template<typename ConnectionFactory>
void AsyncServer<ConnectionFactory>::stop()
{
    ioService_.stop();
}

template<typename ConnectionFactory>
void AsyncServer<ConnectionFactory>::setErrorHandler(error_handler handler)
{
    errorHandler_ = handler;
}

template<typename ConnectionFactory>
void AsyncServer<ConnectionFactory>::listen(
    const boost::asio::ip::tcp::endpoint& bindPort
)
{
    acceptor_.open(bindPort.protocol());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(bindPort);
    acceptor_.listen();
}

template<typename ConnectionFactory>
void AsyncServer<ConnectionFactory>::asyncAccept()
{
    newConnection_.reset(
        connectionFactory_->create(ioService_)
    );
    acceptor_.async_accept(
        newConnection_->socket(),
        boost::bind(
            &AsyncServer<ConnectionFactory>::onAccept,
            this,
            boost::asio::placeholders::error
        )
    );

}

template<typename ConnectionFactory>
void AsyncServer<ConnectionFactory>::onAccept(const boost::system::error_code& e)
{
    if (!e)
    {
        newConnection_->start();

        asyncAccept();
    }
    else
    {
        onError(e);
    }
}

template<typename ConnectionFactory>
void AsyncServer<ConnectionFactory>::onError(const boost::system::error_code& e)
{
    if (errorHandler_)
    {
        errorHandler_(e);
    }
}

} // namespace sf1v5

#endif // NET_TCPSERVER_ASYNC_SERVER_H
