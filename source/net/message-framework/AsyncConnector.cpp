#include <net/message-framework/AsyncConnector.h>

#include <iostream>

using boost::asio::ip::tcp;

namespace messageframework
{
	AsyncConnector::AsyncConnector(AsyncStreamFactory* streamFactory, boost::asio::io_service& ioservice):
			io_service_(ioservice),
			streamFactory_(streamFactory)
	{
	}

	AsyncConnector::~AsyncConnector()
	{
			DLOG(INFO)  << "delete streams...";

			for (std::list<AsyncStream* >::iterator iter = streams_.begin();
							iter != streams_.end(); iter++) {
					delete *iter; //delete streams and connections & interfaces
			}

			DLOG(INFO)  << "finish shutdown...";
	}


	size_t AsyncConnector::streamNum(void)
	{
		return streams_.size();
	}

	void AsyncConnector::closeStream(AsyncStream* stream)
	{
		streams_.remove(stream);
	}

	void AsyncConnector::shutdown(void)
	{
			DLOG(INFO) << "close acceptors...";

			for (std::list<boost::shared_ptr<tcp::acceptor> >::iterator iter0 = acceptors_.begin();
							iter0 != acceptors_.end(); iter0++) {
					(*iter0)->close(); //close acceptors
			}

			DLOG(INFO) << "shutdown streams...";

			for (std::list<AsyncStream* >::iterator iter = streams_.begin();
							iter != streams_.end(); iter++) {
					(*iter)->shutdown(); //shutdown stream
			}

			DLOG(INFO) << "finish shutdown...";
	}

	void AsyncConnector::listen(int port)
	{
		tcp::endpoint endpoint(tcp::v4(), port);
		boost::shared_ptr<tcp::acceptor> acceptor(new tcp::acceptor(io_service_, endpoint));
		acceptor->listen();
		acceptors_.push_back(acceptor);

		DLOG(INFO) << "Listen at : " << port;

		boost::shared_ptr<tcp::socket> sock(new tcp::socket(io_service_));
		acceptor->async_accept(*sock, boost::bind(&AsyncConnector::handle_accept, this,
								acceptor, sock, boost::asio::placeholders::error));
	}


	void AsyncConnector::handle_accept(boost::shared_ptr<tcp::acceptor> acceptor,
				boost::shared_ptr<tcp::socket> sock,
				const boost::system::error_code& error)
	{
		if (!error)
		{
			DLOG(INFO) << "Accept new connection";

			AsyncStream* stream = streamFactory_->createAsyncStream(sock);
			streams_.push_back(stream);
			boost::shared_ptr<tcp::socket> new_sock(new tcp::socket(io_service_));
			acceptor->async_accept(*new_sock, boost::bind(&AsyncConnector::handle_accept, this,
					acceptor, new_sock, boost::asio::placeholders::error));
		} else {
			DLOG(INFO) << "Connection closed while accepting";
		}
	}

	void AsyncConnector::connect(const std::string& host, unsigned int port)
	{
		std::stringstream sstream;
		sstream << port;
		std::string strPort = sstream.str();

		connect(host, strPort);
	}

	void AsyncConnector::connect(const std::string& host, const std::string& port)
	{

		tcp::resolver resolver(io_service_);
		tcp::resolver::query query(tcp::v4(), host, port);
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		tcp::endpoint endpoint = *endpoint_iterator;

		DLOG(INFO) << "Connect to " << host << ":" << port;

		boost::shared_ptr<tcp::socket> sock(new tcp::socket(io_service_));
		sock->async_connect(endpoint,
			boost::bind(&AsyncConnector::handle_connect, this, sock,
							boost::asio::placeholders::error,
							++endpoint_iterator));
	}

	void AsyncConnector::handle_connect(boost::shared_ptr<tcp::socket> sock,
					const boost::system::error_code& error,
					tcp::resolver::iterator endpoint_iterator)
	{
		if (!error)
		{
			DLOG(INFO) << "Connection is established";
			AsyncStream* stream = streamFactory_->createAsyncStream(sock);
			streams_.push_back(stream);
		}
		else if (endpoint_iterator != tcp::resolver::iterator())
		{
			DLOG(WARNING) << "Fail to connect to server, try another endpoint";
			sock->close();
			tcp::endpoint endpoint = *endpoint_iterator;
			sock->async_connect(endpoint,
					boost::bind(&AsyncConnector::handle_connect, this, sock,
						boost::asio::placeholders::error, ++endpoint_iterator));
		}
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

