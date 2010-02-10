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


    template<typename AsyncStreamManager>
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
            AsyncStreamManager& streamManager)
        :   io_service_(ioservice),
            streamManager_(streamManager) {}

        /**
          * @brief Default destructor
          */
        ~AsyncAcceptor() {};

        /**
          * @brief start listening at a given port. If it accepts new connection, AsyncStreamFactory
          * is used to create new AsyncStream.
          * @param port listening port
          */
        void listen(int port)
        {
            tcp::endpoint endpoint(tcp::v4(), port);
            boost::shared_ptr<tcp::acceptor> acceptor(new tcp::acceptor(io_service_, endpoint));
            acceptor->listen();
            acceptors_.push_back(acceptor);

            DLOG(INFO) << "Listen at : " << port;

            boost::shared_ptr<tcp::socket> sock(new tcp::socket(io_service_));
            acceptor->async_accept(*sock, boost::bind(&AsyncAcceptor::handle_accept, this,
                                    acceptor, sock, boost::asio::placeholders::error));
        }

        /**
          * @brief Shutdown all streams and acceptors
          */
        void shutdown(void)
        {
			DLOG(INFO) << "close acceptors...";

			for (std::list<boost::shared_ptr<tcp::acceptor> >::iterator iter0 = acceptors_.begin();
							iter0 != acceptors_.end(); iter0++) {
					(*iter0)->close(); //close acceptors
			}
        }

    private:
        /**
          * @brief this function is called when accepting a new connection
          */
        void handle_accept(boost::shared_ptr<tcp::acceptor> acceptor,
                           boost::shared_ptr<tcp::socket> sock,
                           const boost::system::error_code& error)
        {
            if (!error)
            {
                DLOG(INFO) << "Accept new connection";

                streamManager_.addStream(sock);
                boost::shared_ptr<tcp::socket> new_sock(new tcp::socket(io_service_));
                acceptor->async_accept(*new_sock, boost::bind(&AsyncAcceptor::handle_accept, this,
                        acceptor, new_sock, boost::asio::placeholders::error));
            } else {
                DLOG(INFO) << "Connection closed while accepting";
            }
        }

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
    template<typename AsyncStreamManager>
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

        virtual ConnectionFuture doConnect(const std::string& host, const std::string& port)
        {
            ConnectionFuture connectionFuture(host, port);

            tcp::resolver resolver(io_service_);
            tcp::resolver::query query(tcp::v4(), host, port);
            tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
            tcp::endpoint endpoint = *endpoint_iterator;

            DLOG(INFO) << "Connect to " << host << ":" << port;

            boost::shared_ptr<tcp::socket> sock(new tcp::socket(io_service_));
            sock->async_connect(endpoint, boost::bind(&AsyncConnector::handle_connect,
                this, sock, ++endpoint_iterator, connectionFuture,
                    boost::asio::placeholders::error));
            return connectionFuture;
        }

    private:

        /**
          * @brief this function is called when connection to a remote socket
          * is successful.
          */
        void handle_connect(boost::shared_ptr<tcp::socket> sock,
                            tcp::resolver::iterator endpoint_iterator,
                            ConnectionFuture connectionFuture,
                            const boost::system::error_code& error)
        {
            if (!error)
            {
                DLOG(INFO) << "Connection is established";
                connectionFuture.setStatus(true, true);
                streamManager_.addStream(sock);
            }
            else if (endpoint_iterator != tcp::resolver::iterator())
            {
                DLOG(WARNING) << "Fail to connect to server, try another endpoint";
                sock->close();
                tcp::endpoint endpoint = *endpoint_iterator;
                sock->async_connect(endpoint, boost::bind(&AsyncConnector::handle_connect,
                    this, sock, ++endpoint_iterator, connectionFuture,
                        boost::asio::placeholders::error));
            } else {
                DLOG(WARNING) << "Fail to connect to server";
                connectionFuture.setStatus(true, false);
            }
        }

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
    template<typename AsyncStreamManager>
    class AsyncControllerConnector : public AsyncConnector<AsyncStreamManager>
    {
    public:

        AsyncControllerConnector(boost::asio::io_service& ioservice,
            AsyncStreamManager& streamManager, int check_interval)
        :   AsyncConnector<AsyncStreamManager>(ioservice, streamManager),
                check_interval_(check_interval),
                    connect_check_handler_ (ioservice) {}

        /**
          * @brief Default destructor
          */
        virtual ~AsyncControllerConnector() {}

        void stop() {connect_check_handler_.cancel();}

    protected:

        ConnectionFuture doConnect(const std::string& host, const std::string& port)
        {
            ConnectionFuture cf = AsyncConnector<AsyncStreamManager>::doConnect(host, port);

            connect_check_handler_.expires_from_now(
                boost::posix_time::seconds(check_interval_));
            connect_check_handler_.async_wait(boost::bind(
                &AsyncControllerConnector<AsyncStreamManager>::controllerConnectionCheckHandler,
                    this, check_interval_, cf, boost::asio::placeholders::error) );
            return cf;
        }

    private:

        /**
         * @brief Check connection to controller from time to time
         */
		void controllerConnectionCheckHandler(const int check_interval,
            const std::string host, const std::string port,
                const boost::system::error_code& error)
        {
            if (!error)
            {
                connect_check_handler_.expires_from_now(
                    boost::posix_time::seconds(check_interval));

                if ( !AsyncConnector<AsyncStreamManager>::streamManager_.exist(MessageFrameworkNode(host,
                    boost::lexical_cast<unsigned int>(port) )))
                {
                    DLOG(WARNING) << "Try to reconnect to the controller";
                    ConnectionFuture cf = AsyncConnector<AsyncStreamManager>::doConnect( host, port);
                    connect_check_handler_.async_wait(boost::bind(
                        &AsyncControllerConnector<AsyncStreamManager>::controllerConnectionCheckHandler,
                            this, check_interval, cf,
                                boost::asio::placeholders::error));
                } else {
                    connect_check_handler_.async_wait(boost::bind(
                        &AsyncControllerConnector<AsyncStreamManager>::controllerConnectionCheckHandler,
                            this, check_interval, host, port,
                                boost::asio::placeholders::error));
                }
            }
        }

        /**
         * @brief Check connection to controller from time to time
         */
		void controllerConnectionCheckHandler(const int check_interval,
                ConnectionFuture cf, const boost::system::error_code& error)
        {
            if (!error)
            {
                connect_check_handler_.expires_from_now(
                    boost::posix_time::seconds(check_interval));

                // fail to connect last time
                if ( cf.isFinish() && !cf.isSucc() ) {
                    DLOG(WARNING) << "Try to reconnect to the controller";
                    std::string host = cf.getHost();
                    std::string port = cf.getPort();
                    ConnectionFuture cf = AsyncConnector<AsyncStreamManager>::doConnect(host , port);
                    connect_check_handler_.async_wait(boost::bind(
                        &AsyncControllerConnector<AsyncStreamManager>::controllerConnectionCheckHandler,
                            this, check_interval, cf,
                                boost::asio::placeholders::error));
                } else {
                    connect_check_handler_.async_wait(boost::bind(
                        &AsyncControllerConnector<AsyncStreamManager>::controllerConnectionCheckHandler,
                            this, check_interval, cf.getHost(), cf.getPort(),
                                boost::asio::placeholders::error));
                }
            }
        }


        int check_interval_;

        /**
         * @brief timer which fires thread to check connection to
         * controller every few seconds.
         */
		boost::asio::deadline_timer connect_check_handler_;
    };


    template<typename Application, typename MessageDispatcherType>
    class AsyncStreamManager
    {
    typedef AsyncStream<Application, MessageDispatcherType> AsyncStreamType;

    public:
        AsyncStreamManager(MessageDispatcherType& dispatcher)
            : messageDispatcher_(dispatcher) {}

        ~AsyncStreamManager()
        {
                DLOG(INFO)  << "delete streams...";
                for (typename std::list<AsyncStreamType* >::iterator iter = streams_.begin();
                                iter != streams_.end(); iter++) {
                        delete *iter; //delete streams and connections & interfaces
                }
        }

        /**
         * @brief Add a socket as stream
         */
        void addStream(boost::shared_ptr<tcp::socket> socket)
        {
            streams_.push_back(new AsyncStreamType(&messageDispatcher_, socket));
        }

        /**
         * @brief Does stream exist
         */
        bool exist(const MessageFrameworkNode& node)
        {
            return messageDispatcher_.isExist(node);
        }

        /**
         * @brief Return the number of AsyncStream
         */
        size_t streamNum(void)
        {
            return streams_.size();
        }


        /**
          * @brief Shutdown all streams
          */
        void shutdown(void)
        {
            DLOG(INFO) << "shutdown streams...";

            for (typename std::list<AsyncStreamType* >::iterator iter = streams_.begin();
                iter != streams_.end(); iter++) {
                (*iter)->shutdown(); //shutdown stream
            }
        }

    private:

        /**
         * @brief MessageDispatcher is essential for creating a AsyncStream object
         */
        MessageDispatcherType& messageDispatcher_;

        /**
          * @brief A list of AsyncStream that has been accepted
          */
        std::list<AsyncStreamType* > streams_;
    };

    std::string getHostIp(boost::asio::io_service& ioservice, const std::string&  host)	;

    std::string getLocalHostIp(boost::asio::io_service& ioservice);

}// end of namespace messageframework

#endif // #ifndef _ASYNC_CONNECTOR_H_
