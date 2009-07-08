#ifdef _LOGGING_
#include <net/message-framework/common.h>
#endif

#include <net/message-framework/MessageType.h>
#include <net/message-framework/MessageDispatcher.h>
#include <net/message-framework/AsyncStream.h>

#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <deque>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/function.hpp>
#include <boost/asio.hpp>

namespace messageframework {

long AsyncStream::sended_data_size = 0;

AsyncStream::AsyncStream() {
	writing_ = false;
}

AsyncStream::AsyncStream(MessageDispatcher* messageDispatcher,
		boost::shared_ptr<tcp::socket> sock) :
	socket_(sock), messageDispatcher_(messageDispatcher) {
	writing_ = false;

	// tcp::endpoint endpoint = sock->local_endpoint();
	tcp::endpoint endpoint = sock->remote_endpoint();
	peerNode_.nodeIP_ = endpoint.address().to_string();
	peerNode_.nodePort_ = endpoint.port();
	messageDispatcher_->addPeerNode(peerNode_, this);
	start();
}

AsyncStream::~AsyncStream() {
	if (socket_->is_open())
		socket_->close();
}

void AsyncStream::shutdown(void) {
	if (socket_->is_open())
		socket_->close();
}

void AsyncStream::start() {
	readNewMessage();
}

void AsyncStream::readNewMessage() {
	// read next message
	boost::asio::async_read(*socket_, boost::asio::buffer(&readHeaderBufer_,
			sizeof(unsigned int)), boost::bind(&AsyncStream::readMessageData,
			this, boost::asio::placeholders::error));
}

void AsyncStream::readMessageData(const boost::system::error_code& error) {
	if (!error) {

#ifdef _LOGGING_
		WriteToLog("log.log", "Received message header " );
#endif
		// The message header consist of 4 bytes
		// the first byte contains the message type
		// the next 3 bytes contain the data length
		messageType_ = MessageType((readHeaderBufer_ >> 28)& 0xFF);
		dataLength_ = readHeaderBufer_ & 0x00FFFFFF;
		
#ifdef 	SF1_DEBUG
		cout<<"DBG:: AsyncStream::readMessageData len="<<dataLength_<<endl;
#endif		
		
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
		std::cout
				<< "Error while Receiving message header. Connection is closed."
				<< std::endl;
		std::cout << "Error detail " << error.message() << std::endl;
		messageDispatcher_->removePeerNode(peerNode_);
	}
}

void AsyncStream::handle_read(const boost::system::error_code& error) {
	if (!error) {
#ifdef _LOGGING_
		WriteToLog("log.log", "Received message data " );
#endif
		readBuffer_->commit(dataLength_);		
		messageDispatcher_->sendDataToUpperLayer(peerNode_, messageType_,
				readBuffer_);
		readBuffer_.reset();
		// read next message
		readNewMessage();
	} else {
		std::cout
				<< "Error while Receiving message body. Connection is closed."
				<< std::endl;
		std::cout << "Error detail " << error.message() << std::endl;
		messageDispatcher_->removePeerNode(peerNode_);
	}
}

void AsyncStream::sendMessage(MessageType messageType, //const std::string& data)
		boost::shared_ptr<izenelib::util::izene_streambuf>& buffer) {
	unsigned int dataLength = buffer->size();	
	if ((dataLength >> 28) != 0)
		throw MessageFrameworkException(SF1_MSGFRK_DATA_OUT_OF_RANGE,
				__LINE__, __FILE__);

	sended_data_size += dataLength;
	
#ifdef 	SF1_DEBUG
	cout<<"DBG:: AsyncStream::sendMessage len="<<dataLength<<endl;
#endif
	
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
				cout << "severe error " << "package cannot be sent out" << endl;
				return;
			}
		}
		writing_ = true;
	}
	boost::asio::async_write(*socket_, buffers, boost::bind(
			&AsyncStream::handle_write, this, buffer, writeHeaderBuffer,
			boost::asio::placeholders::error));

}


void AsyncStream::sendMessage(MessageType messageType, const boost::shared_ptr<ServiceMessage>& serviceMessage) {

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
					cout << "severe error " << "package cannot be sent out"
							<< endl;
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

void AsyncStream::handle_write(
		boost::shared_ptr<izenelib::util::izene_streambuf> buffer,
		boost::shared_ptr<std::string> writeHeaderBuffer,
		const boost::system::error_code& error) {
	{
		boost::mutex::scoped_lock lock(writingMutex_);
		writing_ = false;
		writingEvent_.notify_all();
	}

	if (!error) {
#ifdef _LOGGING_
		WriteToLog("log.log", "Finish sending message data(not ServiceMessage) " );
#endif
	} else {
		std::cout << "Error while writting data. Connection is closed."
				<< std::endl;
		std::cout << "Error detail " << error.message() << std::endl;
		messageDispatcher_->removePeerNode(peerNode_);
	}
}

void AsyncStream::	 handle_write1(
			boost::shared_ptr<std::string> writeHeaderBuffer,
		   boost::shared_ptr<std::vector<boost::asio::const_buffer> > a,
			ServiceMessagePtr serviceMessage,			
			const boost::system::error_code& error) {

	{
		boost::mutex::scoped_lock lock(writingMutex_);
		writing_ = false;
		writingEvent_.notify_all();
	}

	if (!error) {
#ifdef _LOGGING_
		WriteToLog("log.log", "Finish sending message data(MessageService) " );
		cout<<"Finish sending message data(MessageService)"<<endl;
#endif
	} else {
		std::cout << "Error while writting data. Connection is closed."
				<< std::endl;
		std::cout << "Error detail " << error.message() << std::endl;
		messageDispatcher_->removePeerNode(peerNode_);
	}
}

}// end of namespace messageframework
