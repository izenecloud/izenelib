#include <net/message-framework/AsyncConnector.h>
#include <net/message-framework/MessageDispatcher.h>

#include <iostream>

using boost::asio::ip::tcp;

namespace messageframework
{
	AsyncAcceptor::AsyncAcceptor( boost::asio::io_service& ioservice,
        AsyncStreamManager& streamManager)
    :   io_service_(ioservice),
            streamManager_(streamManager) {}

	void AsyncAcceptor::shutdown(void)
	{
			DLOG(INFO) << "close acceptors...";

			for (std::list<boost::shared_ptr<tcp::acceptor> >::iterator iter0 = acceptors_.begin();
							iter0 != acceptors_.end(); iter0++) {
					(*iter0)->close(); //close acceptors
			}
	}


	void AsyncAcceptor::listen(int port)
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

	void AsyncAcceptor::handle_accept(boost::shared_ptr<tcp::acceptor> acceptor,
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

	ConnectionFuture AsyncConnector::doConnect(const std::string& host, const std::string& port)
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

	void AsyncConnector::handle_connect(boost::shared_ptr<tcp::socket> sock,
            tcp::resolver::iterator endpoint_iterator, ConnectionFuture connectionFuture,
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

	ConnectionFuture AsyncControllerConnector::doConnect(
        const std::string& host, const std::string& port)
	{
        ConnectionFuture cf = AsyncConnector::doConnect(host, port);

        connect_check_handler_.expires_from_now(
            boost::posix_time::seconds(check_interval_));
        connect_check_handler_.async_wait(boost::bind(
            &AsyncControllerConnector::controllerConnectionCheckHandler,
                this, check_interval_, cf, boost::asio::placeholders::error) );
        return cf;
	}

    void AsyncControllerConnector::controllerConnectionCheckHandler(
        const int check_interval, const std::string host, const std::string port,
                const boost::system::error_code& error)
    {
        if (!error)
        {
            connect_check_handler_.expires_from_now(
                boost::posix_time::seconds(check_interval));

            if ( !streamManager_.exist(MessageFrameworkNode(host,
                boost::lexical_cast<unsigned int>(port) )))
            {
                DLOG(WARNING) << "Try to reconnect to the controller";
                ConnectionFuture cf = AsyncConnector::doConnect( host, port);
                connect_check_handler_.async_wait(boost::bind(
                    &AsyncControllerConnector::controllerConnectionCheckHandler,
                        this, check_interval, cf,
                            boost::asio::placeholders::error));
            } else {
                connect_check_handler_.async_wait(boost::bind(
                    &AsyncControllerConnector::controllerConnectionCheckHandler,
                        this, check_interval, host, port,
                            boost::asio::placeholders::error));
            }
        }
    }

    void AsyncControllerConnector::controllerConnectionCheckHandler(
        const int check_interval, ConnectionFuture cf,
            const boost::system::error_code& error)
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
                ConnectionFuture cf = AsyncConnector::doConnect(host , port);
                connect_check_handler_.async_wait(boost::bind(
                    &AsyncControllerConnector::controllerConnectionCheckHandler,
                        this, check_interval, cf,
                            boost::asio::placeholders::error));
            } else {
                connect_check_handler_.async_wait(boost::bind(
                    &AsyncControllerConnector::controllerConnectionCheckHandler,
                        this, check_interval, cf.getHost(), cf.getPort(),
                            boost::asio::placeholders::error));
            }
        }
    }

	AsyncStreamManager::~AsyncStreamManager()
	{
			DLOG(INFO)  << "delete streams...";
			for (std::list<AsyncStream* >::iterator iter = streams_.begin();
							iter != streams_.end(); iter++) {
					delete *iter; //delete streams and connections & interfaces
			}
	}

	void AsyncStreamManager::addStream(boost::shared_ptr<tcp::socket> socket)
	{
		streams_.push_back(new AsyncStream(&messageDispatcher_, socket));
	}

    bool AsyncStreamManager::exist(const MessageFrameworkNode& node)
    {
        return messageDispatcher_.isExist(node);
    }

	void AsyncStreamManager::shutdown(void)
	{
			DLOG(INFO) << "shutdown streams...";

			for (std::list<AsyncStream* >::iterator iter = streams_.begin();
							iter != streams_.end(); iter++) {
					(*iter)->shutdown(); //shutdown stream
			}
	}

	size_t AsyncStreamManager::streamNum(void)
	{
		return streams_.size();
	}


	std::string getHostIp(boost::asio::io_service& ioservice, const std::string&  host)
	{
		// get the IP of the machine
		tcp::resolver resolver(ioservice);
		tcp::resolver::query query(tcp::v4(), host, "0");
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		tcp::endpoint endpoint = *endpoint_iterator;
		return endpoint.address().to_string();
	}

	std::string getLocalHostIp(boost::asio::io_service& ioservice)
	{
		// get the IP of the machine
		std::string host = boost::asio::ip::host_name();
		tcp::resolver resolver(ioservice);
		tcp::resolver::query query(tcp::v4(), host, "0");
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		tcp::endpoint endpoint = *endpoint_iterator;
		return endpoint.address().to_string();
	}

}// end of namespace messageframework

