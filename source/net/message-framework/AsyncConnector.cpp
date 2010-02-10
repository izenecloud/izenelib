#include <net/message-framework/AsyncConnector.h>

#include <iostream>

using boost::asio::ip::tcp;

namespace messageframework
{

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

