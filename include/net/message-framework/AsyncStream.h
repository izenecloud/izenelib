#ifndef _ASYNC_STREAM_H_
#define _ASYNC_STREAM_H_

#include <net/message-framework/PermissionRequester.h>
#include <net/message-framework/PermissionServer.h>
#include <net/message-framework/ServiceResultRequester.h>
#include <net/message-framework/ServiceResultServer.h>
#include <net/message-framework/ServiceRegistrationServer.h>
#include <net/message-framework/ServiceRegistrationRequester.h>
//#include <message-framework/MFSocketBuffer.h>
#include <net/message-framework/MFSerialization.h>
#include <net/message-framework/MessageType.h>

#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>

#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <deque>

using boost::asio::ip::tcp;

namespace messageframework {

class MessageDispatcher;

/**
 * @brief This classes sends/receives raw data using boost::asio.
 * AsyncStream connects with application layer throught a MessageDispatcher
 * When AsyncStream receives raw data from network, it forward the raw
 * data to MessageDispatcher. AsyncStream also receives raw data from
 * MessageDispatcher, and then, it sends the raw data to peer node.
 */
class AsyncStream {
public:
	/**
	 * @brief Default constructor
	 */
	AsyncStream();

	/**
	 * @brief create an AsyncStream with given tcp::socket and pointer to
	 * MessageDispatcher.
	 * @param messageDispatcher pointer to MessageDispatcher
	 * @param sock tcp::socket that is binded to the AsyncStream
	 */
	AsyncStream(MessageDispatcher* messageDispatcher,
			boost::shared_ptr<tcp::socket> sock);

	/**
	 * @brief default constructor
	 */
	~AsyncStream();

	/**
	 * @brief send a raw data to network layer. The MessageDispatcher calls
	 * this function in order to send raw data to network layer. Then,
	 * the AsyncStream will sends data to peer node.
	 * @param messageType the message type (the type of data)
	 * @param data the raw data
	 */
	void sendMessage(MessageType messageType, //const std::string& data);
			boost::shared_ptr<izenelib::util::izene_streambuf>& buffer);

	void sendMessage(MessageType messageType, const boost::shared_ptr<ServiceMessage>& serviceMessage);

	void shutdown(void);

private:
	/**
	 * @brief start reading from socket
	 */
	void start();

	void readNewMessage();

	/**
	 * @brief This function is called after data is sent to socket
	 */
	void handle_write(
			// boost::shared_ptr<std::string> writeDataBuffer,
			boost::shared_ptr<izenelib::util::izene_streambuf> buffer,
			boost::shared_ptr<std::string> writeHeaderBuffer,
			const boost::system::error_code& error);

	void handle_write1(
			boost::shared_ptr<std::string> writeHeaderBuffer,
		   boost::shared_ptr<std::vector<boost::asio::const_buffer> >,
			ServiceMessagePtr serviceMessage,
			const boost::system::error_code& error);


	/**
	 * @brief This function is called after receiviing a new message from a peer node
	 */
	void handle_read(const boost::system::error_code& error);

	/**
	 * @brief This function read data of a message. A message has two part:
	 * message type and message data.
	 */
	void readMessageData(const boost::system::error_code& error);

private:
	/**
	 * @brief main socket of the stream
	 */
	boost::shared_ptr<boost::asio::ip::tcp::socket> socket_;

	/**
	 * @brief type of the message
	 */
	MessageType messageType_;

	unsigned int dataLength_;

	/**
	 * @brief reading buffer of the message header
	 */
	unsigned int readHeaderBufer_;

	/**
	 * @brief reading buffer of message data
	 */
	boost::shared_ptr<izenelib::util::izene_streambuf> readBuffer_;

	/**
	 * @brief Modified by Wei Cao, 2009-3-3, IMPORTANT FIX
	 * boost::asio::async_write would call async_write_some multiples times
	 * to write a large package out, so it does not guarantee data continuity,
	 * so we should ensure there are no more than one thread writing.
	 */
	boost::mutex writingMutex_;
	bool writing_;
	boost::condition_variable writingEvent_;

	/**
	 * @brief The peerNode current AsyncStream is binded to
	 */
	MessageFrameworkNode peerNode_;

	/**
	 * @brief this information is used to dispatch (send) data to upper layer
	 */
	MessageDispatcher* messageDispatcher_;

public:
	static long sended_data_size;
};

/**
 * @brief this abstract class defines functions to creates new AsyncStream
 */
class AsyncStreamFactory {
public:
	AsyncStreamFactory() {
	}
	virtual ~AsyncStreamFactory() {
	}
	virtual AsyncStream
			* createAsyncStream(boost::shared_ptr<tcp::socket> sock)=0;
};
}// end of namespace messageframework

#endif // end of #ifndef _ASYNC_STREAM_H_
