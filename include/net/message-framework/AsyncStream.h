#ifndef _ASYNC_STREAM_H_
#define _ASYNC_STREAM_H_

#include <net/message-framework/MFSerialization.h>
#include <net/message-framework/MessageType.h>

#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/function.hpp>
#include <boost/asio.hpp>

#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <deque>

using boost::asio::ip::tcp;

namespace messageframework {

/**
 * @brief This classes sends/receives raw data using boost::asio.
 * AsyncStream connects with application layer throught a MessageDispatcher
 * When AsyncStream receives raw data from network, it forward the raw
 * data to MessageDispatcher. AsyncStream also receives raw data from
 * MessageDispatcher, and then, it sends the raw data to peer node.
 */
template<typename Application,
         typename MessageDispatcherType>
class AsyncStream {

//    typedef MessageDispatcher<Application> MessageDispatcherType;

public:
	/**
	 * @brief Default constructor
	 */
	AsyncStream():writing_(false){};

	/**
	 * @brief create an AsyncStream with given tcp::socket and pointer to
	 * MessageDispatcher.
	 * @param messageDispatcher pointer to MessageDispatcher
	 * @param sock tcp::socket that is binded to the AsyncStream
	 */
	AsyncStream(MessageDispatcherType* messageDispatcher,
        boost::shared_ptr<tcp::socket> sock)
    : socket_(sock), messageDispatcher_(messageDispatcher) {
        writing_ = false;

        // tcp::endpoint endpoint = sock->local_endpoint();
        tcp::endpoint endpoint = sock->remote_endpoint();
        peerNode_.nodeIP_ = endpoint.address().to_string();
        peerNode_.nodePort_ = endpoint.port();
        messageDispatcher_->addPeerNode(peerNode_, this);

        readNewMessage();
    }

	/**
	 * @brief default constructor
	 */
	~AsyncStream() {
	    shutdown();
    }

	void shutdown(void){
        if (socket_->is_open())
            socket_->close();
    }

	/**
	 * @brief send a raw data to network layer. The MessageDispatcher calls
	 * this function in order to send raw data to network layer. Then,
	 * the AsyncStream will sends data to peer node.
	 * @param messageType the message type (the type of data)
	 * @param data the raw data
	 */
	void sendMessage(MessageType messageType, //const std::string& data);
			boost::shared_ptr<izenelib::util::izene_streambuf>& buffer)
    {
        unsigned int dataLength = buffer->size();
        if ((dataLength >> 28) != 0)
            throw MessageFrameworkException(SF1_MSGFRK_DATA_OUT_OF_RANGE,
                    __LINE__, __FILE__);

        DLOG(INFO) << "body length" << dataLength <<endl;

        int header = (messageType << 28) | dataLength;
        boost::shared_ptr<std::string> writeHeaderBuffer(new string((char*)&header, sizeof(header)));

        std::vector<boost::asio::const_buffer> buffers;
        buffers.push_back(boost::asio::buffer(*writeHeaderBuffer));
        boost::asio::streambuf* pbuf =
                dynamic_cast<boost::asio::streambuf* > ( &(*buffer) );
        buffers.push_back(pbuf->data() );
        {
            boost::mutex::scoped_lock lock(writingMutex_);
            boost::system_time const timeout = boost::get_system_time()
                    + boost::posix_time::milliseconds(5000);
            while (writing_) {
                if (!writingEvent_.timed_wait(lock, timeout) ) {
                    DLOG(ERROR) << "package cannot be sent out" << endl;
                    return;
                }
            }
            writing_ = true;
        }

        boost::asio::async_write(*socket_, buffers, boost::bind(
                &AsyncStream::handle_write, this, buffer, writeHeaderBuffer,
                boost::asio::placeholders::error));
    }

	void sendMessage(MessageType messageType, const boost::shared_ptr<ServiceMessage>& serviceMessage)
    {

		unsigned int dataLength = serviceMessage->getSerializedSize();
		if ((dataLength >> 28) != 0)
			throw MessageFrameworkException(SF1_MSGFRK_DATA_OUT_OF_RANGE,
					__LINE__, __FILE__);

		int header = (messageType << 28) | dataLength;
		boost::shared_ptr<std::string> writeHeaderBuffer(new string((char*)&header, sizeof(header)));
		unsigned int tl = 0;

		boost::shared_ptr<std::vector<boost::asio::const_buffer> > buffers(new std::vector<boost::asio::const_buffer>);
		buffers->push_back(boost::asio::buffer(*writeHeaderBuffer));
		//buffers.push_back(boost::asio::buffer(&header, sizeof(int)));
		tl += sizeof(MessageHeader);
		buffers->push_back(boost::asio::buffer(&(serviceMessage->mh),
				sizeof(MessageHeader)));
		tl += serviceMessage->getServiceName().size();
		buffers->push_back(boost::asio::buffer(serviceMessage->getServiceName().c_str(), serviceMessage->getServiceName().size()));
		for (unsigned int i=0; i<serviceMessage->getBufferNum(); i++) {
			buffers->push_back(boost::asio::buffer(&(serviceMessage->getBuffer(i)->size), sizeof(size_t)) );
			tl += sizeof(size_t);
			tl += serviceMessage->getBuffer(i)->size;
			buffers->push_back(boost::asio::buffer(serviceMessage->getBuffer(i)->getData(), serviceMessage->getBuffer(i)->size) );
		}

        assert(tl == dataLength);
		// forward data to network layer, the data will be sent
		// to destination
		{
			boost::mutex::scoped_lock lock(writingMutex_);
			boost::system_time const timeout = boost::get_system_time()
					+ boost::posix_time::milliseconds(5000);
			while (writing_) {
				if (!writingEvent_.timed_wait(lock, timeout) ) {
                    DLOG(ERROR) << "package cannot be sent out" << endl;
					return;
				}
			}
		}
		writing_ = true;
		boost::asio::async_write(*socket_, *buffers, boost::bind(
				&AsyncStream::handle_write1, this, writeHeaderBuffer, buffers, serviceMessage,

				boost::asio::placeholders::error)
		);
	}




private:

	/**
	 * @brief start reading from socket
	 */
	void readNewMessage() {
        boost::asio::async_read(*socket_, boost::asio::buffer(&readHeaderBufer_,
                sizeof(unsigned int)), boost::bind(&AsyncStream::readMessageData,
                this, boost::asio::placeholders::error));
    }

	/**
	 * @brief This function read data of a message. A message has two part:
	 * message type and message data.
	 */
	void readMessageData(const boost::system::error_code& error) {
        if (!error) {

            DLOG(INFO) << "Received message header";
            // The message header consist of 4 bytes
            // the first byte contains the message type
            // the next 3 bytes contain the data length
            messageType_ = MessageType((readHeaderBufer_ >> 28)& 0xFF);
            dataLength_ = readHeaderBufer_ & 0x00FFFFFF;

            DLOG(INFO) << "header length "<< dataLength_;

            //readBuffer_.reset(new boost::asio::streambuf());
            readBuffer_.reset(new izenelib::util::izene_streambuf);

            //boost::asio::streambuf::mutable_buffers_type buf =
            //		readBuffer_->prepare(dataLength_);
            izenelib::util::izene_streambuf::mutable_buffers_type buf =
                    readBuffer_->prepare(dataLength_);
            boost::asio::async_read(*socket_, buf, boost::bind(
                    &AsyncStream::handle_read, this,
                    boost::asio::placeholders::error));
        } else {
            DLOG(INFO) << "Connection is closed while waiting for new message : " << error.message();
            messageDispatcher_->removePeerNode(peerNode_);
        }
    }

	/**
	 * @brief This function is called after receiviing a new message from a peer node
	 */
	void handle_read(const boost::system::error_code& error) {
        if (!error) {
            DLOG(INFO) << "Received message data";

            readBuffer_->commit(dataLength_);
            messageDispatcher_->sendDataToUpperLayer(peerNode_, messageType_,
                    readBuffer_);
            readBuffer_.reset();
            // read next message
            readNewMessage();
        } else {
            DLOG(ERROR) << "Connection is closed while receiving message body : " << error.message();
            messageDispatcher_->removePeerNode(peerNode_);
        }
    }


	/**
	 * @brief This function is called after data is sent to socket
	 */
	void handle_write(
			// boost::shared_ptr<std::string> writeDataBuffer,
			boost::shared_ptr<izenelib::util::izene_streambuf> buffer,
			boost::shared_ptr<std::string> writeHeaderBuffer,
			const boost::system::error_code& error)
    {
        {
            boost::mutex::scoped_lock lock(writingMutex_);
            writing_ = false;
            writingEvent_.notify_all();
        }

        if (!error) {
            DLOG(INFO) << "Finish sending message data(MessageService)";
        } else {
            DLOG(ERROR) << " Connection is closed while writting data" << error.message();
            messageDispatcher_->removePeerNode(peerNode_);
        }
    }

	void handle_write1(
			boost::shared_ptr<std::string> writeHeaderBuffer,
		   boost::shared_ptr<std::vector<boost::asio::const_buffer> >,
			ServiceMessagePtr serviceMessage,
			const boost::system::error_code& error)
    {
        {
            boost::mutex::scoped_lock lock(writingMutex_);
            writing_ = false;
            writingEvent_.notify_all();
        }

        if (!error) {
            DLOG(INFO) << "Finish sending message data(MessageService)";
        } else {
            DLOG(ERROR) << " Connection is closed while writting data" << error.message();
            messageDispatcher_->removePeerNode(peerNode_);
        }
    }


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
	MessageDispatcherType* messageDispatcher_;
};

}// end of namespace messageframework

#endif // end of #ifndef _ASYNC_STREAM_H_
