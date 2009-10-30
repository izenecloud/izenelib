///
///  @file : MessageDispatcher.h
///  @date : 20/11/2008
///  @author : TuanQuang Nguyen
///  @brief This file define MessageDispatcher that reponsible for sending
///  data using streams
///

#ifndef _MESSAGE_DISPATCHER_H
#define _MESSAGE_DISPATCHER_H

#include <net/message-framework/MessageFrameworkNode.h>
#include <net/message-framework/ServiceRegistrationRequester.h>
#include <net/message-framework/ServiceRegistrationServer.h>
#include <net/message-framework/PermissionRequester.h>
#include <net/message-framework/PermissionServer.h>
#include <net/message-framework/ServiceResultRequester.h>
#include <net/message-framework/ServiceResultServer.h>
#include <net/message-framework/AsyncStream.h>
#include <net/message-framework/MessageType.h>
#include <net/message-framework/ClientIdRequester.h>
#include <net/message-framework/ClientIdServer.h>
#include <net/message-framework/ThreadPool.h>

#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <map>

namespace messageframework {
/**
 * @brief This class plays an intermediary role between network layer
 * and application layer. It converts raw data sent by network layer
 * into object data, and then, sends the object to application layer.
 * It also converts object data sent by application layer into raw data,
 * and then, sends the raw data to newtwork layer.
 **/
class MessageDispatcher {
public:
	long serialization_time;
	long deserialization_time;
	/**
	 * @brief construct a MessageDispatcher that are used by PermissionRequester
	 * ,ServiceResultRequester and ClientIdRequester
	 * @param permissionRequester pointer to PermissionRequester
	 * @param resultRequester pointer to ServiceResultRequester
	 * @param clientIdRequester pointer to ClientIdRequester
	 */
	MessageDispatcher(PermissionRequester* permissionRequester,
			ServiceResultRequester* resultRequester,
			ClientIdRequester* clientIdRequester);

	/**
	 * @brief construct a MessageDispatcher that are used by ServiceRegistrationRequester
	 * and ServiceResultServer
	 * @param registrationRequester pointer to ServiceRegistrationRequester
	 * @param resultServer pointer to ServiceResultServer
	 */
	MessageDispatcher(ServiceRegistrationRequester* registrationRequester,
			ServiceResultServer* resultServer);

	/**
	 * @brief construct a MessageDispatcher that are used by ServiceRegistrationServer,
	 * PermissionServer, ServiceResultClient, ServiceResultServer, and ClientIdServer
	 * @param registrationServer pointer to ServiceRegistrationServer
	 * @param permissionServer pointer to PermissionServer
	 * @param resultClient pointer to ServiceResultClient
	 * @param resultServer pointer to ServiceResultServer
	 * @param clientIdServer pointer to ClientIdServer
	 */
	MessageDispatcher(ServiceRegistrationServer* registrationServer,
			PermissionServer* permissionServer,
			ServiceResultRequester* resultClient,
			ServiceResultServer* resultServer, ClientIdServer* clientIdServer);

	/**
	 * @brief default destructor,
	 */
	~MessageDispatcher();

	bool isExist(const MessageFrameworkNode& node);

	/**
	 * When network layer (AsycnStream class) receives raw data, the network layer calls
	 * this function in order to send data to upper layer. MessageDispatcher converts
	 * raw data into object data according to messageType. The object will be sent
	 * to upper layer.
	 * @param source the source (node) where data is received from
	 * @param messageType the message type
	 * @param data the raw message data
	 */
	void sendDataToUpperLayer(const MessageFrameworkNode& source,
			MessageType messageType, //const std::string& data);
			boost::shared_ptr<izenelib::util::izene_streambuf>& buffer);

#if 1
	bool sendDataToLowerLayer1(MessageType messageType,
			const ServiceMessagePtr& serviceMessage,
			const MessageFrameworkNode& destination) {
		return sendDataToLowerLayer(messageType, *serviceMessage, destination);
	}
#else	 

	bool sendDataToLowerLayer1(
			MessageType messageType,
			const ServiceMessagePtr& data,
			const MessageFrameworkNode& destination) {
		if (threadPool_) {
			ThreadPool::Task task = boost::bind(
					&MessageDispatcher::sendDataToLowerLayer_impl1,
					this, messageType, data, destination );
			threadPool_->executeTask(task);
		}
		else
		return sendDataToLowerLayer_impl1(messageType, data, destination);
	}

#endif
	/**
	 * @brief This function sends a message having messageType and dataType to a
	 * peer node. If threadPool exists,
	 * @param messageType the message type
	 * @param data the message data
	 * @param destination the node where data will be sent to
	 * @return true if the data is sent to peer node
	 * @return false if the node is not found
	 */
	template <typename DataType> bool sendDataToLowerLayer(
			MessageType messageType, const DataType& data,
			const MessageFrameworkNode& destination) {
		if (threadPool_) {
			ThreadPool::Task task = boost::bind(
					&MessageDispatcher::sendDataToLowerLayer_impl<DataType>,
					this, messageType, data, destination );
					threadPool_->executeTask(task);
				}
				else
				return sendDataToLowerLayer_impl<DataType>(messageType, data,destination);
		return true;
		
	}

	/**
	 * @brief Add a new peer node to the available peer list
	 * @param peerNode - the peer node
	 * @param bindedStream - the stream that is binded to the connection with the peer node
	 */
	void addPeerNode(const MessageFrameworkNode& peerNode, AsyncStream* bindedStream);

	/**
	 * @brief Remove a a peer node from the available peer list
	 * @param peerNode the peer node to be removed
	 */
	void removePeerNode(const MessageFrameworkNode& peerNode);

	/**
	 * @brief Use the thread pool for serialization
	 * @param threadPool - the thread pool to be used
	 */
	void attachThreadPool(ThreadPool* threadPool) {
		threadPool_ = threadPool;
	}

private:

	/**
	 * @brief Retrieve a stream that is binded to a node
	 * @param node node where the stream is binded to
	 * @return the stream
	 * @return NULL if the stream does not exist
	 */
	AsyncStream& getStreamByNode(const MessageFrameworkNode& node);

	/**
	 * @brief This function should be called by threads in threadPool_ or called
	 * directly from sendDataToLowerLayer. To avoid arguments be copied when the
	 * funtion object generated by boost::bind copied elsewhere, I change all
	 * arguments to be pointer.
	 */
	template <typename DataType>
	bool sendDataToLowerLayer_impl(MessageType  messageType,
			const DataType& data,
			const MessageFrameworkNode& destination)
	{

		boost::posix_time::ptime before = boost::posix_time::microsec_clock::local_time();
                DLOG(INFO)<< typeid(DataType).name() << "sendDataToLowerLayer_impl...";

		boost::shared_ptr<izenelib::util::izene_streambuf> archive_stream(
				new izenelib::util::izene_streambuf());

		to_buffer(data, archive_stream);

		boost::posix_time::ptime after = boost::posix_time::microsec_clock::local_time();
		serialization_time += (after-before).total_microseconds();

		// retrieve stream that is binded to destination
		AsyncStream& stream = getStreamByNode(destination);
	  
		DLOG(INFO)<<(const char*)archive_stream->data();		

		// forward data to network layer, the data will be sent
		// to destination
		stream.sendMessage(messageType, archive_stream);
		return true;

	}


#if 0	
	bool sendDataToLowerLayer_impl1(MessageType  messageType,
			const ServiceMessagePtr& serviceMessage,
			const MessageFrameworkNode& destination)
		{
						// retrieve stream that is binded to destination
		AsyncStream& stream = getStreamByNode(destination);
#ifdef _LOGGING_
		WriteToLog("log.log", " sendDataToLowerLayer : ServiceMessage ..." );  		
#endif   
		stream.sendMessage(messageType, serviceMessage);

		/* unsigned int dataLength = serviceMessage.getSerializedSize();
		if ((dataLength >> 24) != 0)
		 throw MessageFrameworkException(SF1_MSGFRK_DATA_OUT_OF_RANGE,
				__LINE__, __FILE__);

	   int header = (messageType << 24) | dataLength;	  
	   	boost::shared_ptr<std::string> writeHeaderBuffer(new string((char*)&header, sizeof(header)));

     cout<<"DBG sendDataToLowerLayer_impl 1 : ServiceMessage ..." <<endl;

	std::vector<boost::asio::const_buffer> buffers;
	buffers.push_back(boost::asio::buffer(*writeHeaderBuffer));
	//buffers.push_back(boost::asio::buffer(&header, sizeof(int)));
	buffers.push_back(boost::asio::buffer(&(serviceMessage.mh), sizeof(MessageHeader)));
	buffers.push_back(boost::asio::buffer(serviceMessage.getServiceName().c_str(), 
		serviceMessage.getServiceName().size()));
	for(int i=0; i<serviceMessage.getBufferNum(); i++){
		cout<<i<<endl;	
		unsigned int sz = serviceMessage.getBuffer(i)->getSize();
		cout<< sz<<endl;
		buffers.push_back(boost::asio::buffer(&sz, sizeof(unsigned int)) ); 		
		buffers.push_back(boost::asio::buffer(serviceMessage.getBuffer(i)->getData(), sz));
	}  
		// forward data to network layer, the data will be sent
		// to destination		
		stream.sendMessage(buffers);	*/	

		}
#endif

/**
 * This function should be called by threads in threadPool_ or called
 * directly from sendDataToUpperLayer. To avoid arguments be copied when the
 * funtion object generated by boost::bind copied elsewhere, I change all
 * arguments to be pointer.
 */
void sendDataToUpperLayer_impl(const MessageFrameworkNode& source,
	MessageType messageType, //const std::string& data);
	boost::shared_ptr<izenelib::util::izene_streambuf>& buffer);

private:
/**
 * @brief message's serialization would be executed in the thread pool
 * so that upper thread sending the message or lower io thread receiving
 * the message can return quickly
 */
ThreadPool* threadPool_;

/**
 * @brief list of available peer nodes. There is a AsyncStream
 * binded to each peer node
 **/
std::map<MessageFrameworkNode, AsyncStream*, MessageFrameworkNode_less> availablePeerNodes_;
boost::mutex availablePeerNodesMutex_;

/**
 * @brief Pointer to a ServiceRegistrationRequester
 */
ServiceRegistrationRequester* registrationRequester_;

/**
 * @brief Pointer to a ServiceRegistrationServer
 */
ServiceRegistrationServer* registrationServer_;

/**
 * @brief Pointer to a ServicePermissionRequester
 */
PermissionRequester* permissionRequester_;

/**
 * @brief Pointer to a ServicePermissionServer
 */
PermissionServer* permissionServer_;

/**
 * @brief Pointer to a ServiceResultRequester
 */
ServiceResultRequester* resultRequester_;

/**
 * @brief Pointer to a ServiceResultServer
 */
ServiceResultServer* resultServer_;

/**
 * @brief Pointer to a ClientIdRequster;
 */
ClientIdRequester* clientIdRequester_;

/**
 * @brief Pointer to a ClientIdServer
 */
ClientIdServer* clientIdServer_;

};
}// end of namespace messageframework

#endif


