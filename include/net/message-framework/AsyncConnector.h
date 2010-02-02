#ifndef _ASYNC_CONNECTOR_H_
#define _ASYNC_CONNECTOR_H_

#include <net/message-framework/AsyncStream.h>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

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
        ConnectionFuture() : impl_(new ConnectionFutureImpl()) {}
        ConnectionFuture(const ConnectionFuture& cf) : impl_(cf.impl_) {}
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
            ConnectionFutureImpl() : finish_(false), succ_(false) {}
            bool finish_;
            bool succ_;
            boost::mutex mutex_;
            boost::condition_variable semaphore_;
        };
        boost::shared_ptr<ConnectionFutureImpl> impl_;
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
        AsyncConnector(AsyncStreamFactory* streamFactory, boost::asio::io_service& ioservice);

        /**
          * @brief Default destructor
          */
        ~AsyncConnector();

        /**
          * @brief start listening at a given port. If it accepts new connection, AsyncStreamFactory
          * is used to create new AsyncStream.
          * @param port listening port
          */
        void listen(int port);

        /**
          * @brief connection to a given host:port
          * @param host the host IP
          * @param port the port number
          */
        ConnectionFuture connect(const std::string& host, const std::string& port);

        ConnectionFuture connect(const std::string& host, unsigned int port);

        /**
         * @brief Return the number of AsyncStream
         */
        size_t streamNum(void);

        /**
          * @brief close a given stream
          * @param stream: the stream to close
          */
        void closeStream(AsyncStream* stream);

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


        /**
          * @brief this function is called when connection to a remote socket
          * is successful.
          */
        void handle_connect(boost::shared_ptr<tcp::socket> sock,
                            tcp::resolver::iterator endpoint_iterator,
                            ConnectionFuture connectionFuture,
                            const boost::system::error_code& error);

    private:
        /**
          * @brief I/O operation queue
          */
        boost::asio::io_service& io_service_;

        /**
          * @brief A list of AsyncStream that has been accepted
          */
        std::list<AsyncStream* > streams_;


        std::list<boost::shared_ptr<tcp::acceptor> > acceptors_;


        AsyncStreamFactory* streamFactory_;
    };


    std::string getHostIp(boost::asio::io_service& ioservice, const std::string&  host)	;

    std::string getLocalHostIp(boost::asio::io_service& ioservice);

}// end of namespace messageframework

#endif // #ifndef _ASYNC_CONNECTOR_H_
