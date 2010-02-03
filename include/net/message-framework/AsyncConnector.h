#ifndef _ASYNC_CONNECTOR_H_
#define _ASYNC_CONNECTOR_H_

#include <net/message-framework/AsyncStream.h>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

#include <string>
#include <list>


using boost::asio::ip::tcp;

namespace messageframework
{
    /**
     * @brief Follow the feature design pattern
     */
    class ConnectionFuture
    {
    public:
        ConnectionFuture(std::string host, std::string port)
        : impl_(new ConnectionFutureImpl(host, port)) {}
        ConnectionFuture(const ConnectionFuture& cf)
        : impl_(cf.impl_) {}
        ConnectionFuture& operator = (const ConnectionFuture& cf)
        {
            impl_ = cf.impl_;
            return *this;
        }
        void setStatus(bool finish, bool succ)
        {
            boost::mutex::scoped_lock lock(impl_->mutex_);
            impl_->finish_ = finish;
            impl_->succ_ = succ;
            impl_->semaphore_.notify_all();
        }
        bool isFinish()
        {
            return impl_->finish_;
        }
        bool isSucc()
        {
            return impl_->succ_;
        }
        const std::string& getHost()
        {
            return impl_->host_;
        }
        const std::string& getPort()
        {
            return impl_->port_;
        }
        void wait()
        {
            boost::mutex::scoped_lock lock(impl_->mutex_);
            while(!impl_->finish_) {
                impl_->semaphore_.wait(lock);
            }
        }
        bool timed_wait(boost::system_time const& abs_time)
        {
            boost::mutex::scoped_lock lock(impl_->mutex_);
            while(!impl_->finish_) {
                if(!impl_->semaphore_.timed_wait(lock, abs_time))
                    return false;
            }
            return true;
        }

    private:
        class ConnectionFutureImpl
        {
        public:
            ConnectionFutureImpl(std::string host, std::string port)
            : host_(host), port_(port), finish_(false), succ_(false) {}
            std::string host_;
            std::string port_;
            bool finish_;
            bool succ_;
            boost::mutex mutex_;
            boost::condition_variable semaphore_;
        };
        boost::shared_ptr<ConnectionFutureImpl> impl_;
    };

    class AsyncStreamManager;

    class AsyncAcceptor
    {
    public:
        /**
          * @brief Construct a AsyncConnector with a given AsyncStreamFactory and io_service.
          * The AsyncStreamFactory creates new AsyncStream when it accepts a new connection.
          * @param streamFactory pointer to AsyncStreamFactory
          * @param ioservice the thread for I/O queues
          */
        AsyncAcceptor(boost::asio::io_service& ioservice,
            AsyncStreamManager& streamManager);

        /**
          * @brief Default destructor
          */
        ~AsyncAcceptor() {};

        /**
          * @brief start listening at a given port. If it accepts new connection, AsyncStreamFactory
          * is used to create new AsyncStream.
          * @param port listening port
          */
        void listen(int port);

        /**
          * @brief Shutdown all streams and acceptors
          */
        void shutdown(void);

    private:
        /**
          * @brief this function is called when accepting a new connection
          */
        void handle_accept(boost::shared_ptr<tcp::acceptor> acceptor,
                           boost::shared_ptr<tcp::socket> sock,
                           const boost::system::error_code& error);
    private:
        /**
          * @brief I/O operation queue
          */
        boost::asio::io_service& io_service_;

        std::list<boost::shared_ptr<tcp::acceptor> > acceptors_;

        AsyncStreamManager& streamManager_;
    };

    /**
     * @brief This class control the connection with peer node
     */
    class AsyncConnector
    {
    public:
        /**
          * @brief Construct a AsyncConnector with a given AsyncStreamFactory and io_service.
          * The AsyncStreamFactory creates new AsyncStream when it accepts a new connection.
          * @param streamFactory pointer to AsyncStreamFactory
          * @param ioservice the thread for I/O queues
          */
        AsyncConnector(boost::asio::io_service& ioservice,
            AsyncStreamManager& streamManager)
        : io_service_(ioservice), streamManager_(streamManager) {}

        /**
          * @brief Default destructor
          */
        virtual ~AsyncConnector() {}

        /**
          * @brief connection to a given host:port
          * @param host the host IP
          * @param port the port number
          */

        ConnectionFuture connect(const std::string& host, const unsigned int port)
        {
            return doConnect(host, boost::lexical_cast<std::string>(port));
        }

        ConnectionFuture connect(const MessageFrameworkNode & node)
        {
            return doConnect(node.nodeIP_, boost::lexical_cast<std::string>(node.nodePort_));
        }

    protected:

        virtual ConnectionFuture doConnect(const std::string& host, const std::string& port);

    private:

        /**
          * @brief this function is called when connection to a remote socket
          * is successful.
          */
        void handle_connect(boost::shared_ptr<tcp::socket> sock,
                            tcp::resolver::iterator endpoint_iterator,
                            ConnectionFuture connectionFuture,
                            const boost::system::error_code& error);

    protected:
        /**
          * @brief I/O operation queue
          */
        boost::asio::io_service& io_service_;

        AsyncStreamManager& streamManager_;
    };


    /**
     * @brief This class control the connection with peer node
     */
    class AsyncControllerConnector : public AsyncConnector
    {
    public:

        AsyncControllerConnector(boost::asio::io_service& ioservice,
            AsyncStreamManager& streamManager, int check_interval)
        :   AsyncConnector(ioservice, streamManager),
                check_interval_(check_interval),
                    connect_check_handler_ (ioservice) {}

        /**
          * @brief Default destructor
          */
        virtual ~AsyncControllerConnector() {}

        void stop() {connect_check_handler_.cancel();}

    protected:

        ConnectionFuture doConnect(const std::string& host, const std::string& port);

    private:

        /**
         * @brief Check connection to controller from time to time
         */
		void controllerConnectionCheckHandler(const int check_interval,
            const std::string host, const std::string port,
                const boost::system::error_code& error);

        /**
         * @brief Check connection to controller from time to time
         */
		void controllerConnectionCheckHandler(const int check_interval,
                ConnectionFuture connectionFuture,
                    const boost::system::error_code& error);

        int check_interval_;

        /**
         * @brief timer which fires thread to check connection to
         * controller every few seconds.
         */
		boost::asio::deadline_timer connect_check_handler_;
    };


    class AsyncStreamManager
    {
    public:
        AsyncStreamManager(MessageDispatcher& dispatcher)
            : messageDispatcher_(dispatcher) {}

        ~AsyncStreamManager();

        /**
         * @brief Add a socket as stream
         */
        void addStream(boost::shared_ptr<tcp::socket> socket);

//        /**
//         * @brief Shutdown and destruct stream
//         */
//        void eraseStream(const MessageFrameworkNode& node);

        /**
         * @brief Does stream exist
         */
        bool exist(const MessageFrameworkNode& node);

        /**
         * @brief Return the number of AsyncStream
         */
        size_t streamNum(void);

        /**
          * @brief Shutdown all streams
          */
        void shutdown(void);

    private:

        /**
         * @brief MessageDispatcher is essential for creating a AsyncStream object
         */
        MessageDispatcher& messageDispatcher_;

        /**
          * @brief A list of AsyncStream that has been accepted
          */
        std::list<AsyncStream* > streams_;
    };

    std::string getHostIp(boost::asio::io_service& ioservice, const std::string&  host)	;

    std::string getLocalHostIp(boost::asio::io_service& ioservice);

}// end of namespace messageframework

#endif // #ifndef _ASYNC_CONNECTOR_H_
