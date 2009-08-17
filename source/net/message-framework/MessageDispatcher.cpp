///
///  @file : MessageDispatcher.cpp
///  @date : 20/11/2008
///  @author : TuanQuang Nguyen
///  @brief This file implements MessageDispatcher that reponsible for sending
///  data using streams
///

#include <net/message-framework/MessageDispatcher.h>
#include <net/message-framework/MessageType.h>
#include <net/message-framework/ServiceRegistrationMessage.h>
#include <net/message-framework/ServiceRegistrationReplyMessage.h>
//#include <net/message-framework/PermissionOfServiceMessage.h>

#include <net/message-framework/ServicePermissionInfo.h>
#include <net/message-framework/ServiceMessage.h>
#include <net/message-framework/ClientIdRequestMessage.h>
#include <net/message-framework/ClientIdReplyMessage.h>
#include <boost/archive/archive_exception.hpp>
#include <boost/exception.hpp>
#include <boost/exception/diagnostic_information.hpp>

using namespace std;

namespace messageframework {
MessageDispatcher::MessageDispatcher(PermissionRequester* permissionRequester,
		ServiceResultRequester* resultRequester,
		ClientIdRequester* clientIdRequester) :
	registrationRequester_(NULL), registrationServer_(NULL),
			permissionRequester_(permissionRequester), permissionServer_(NULL),
			resultRequester_(resultRequester), resultServer_(NULL),
			clientIdRequester_(clientIdRequester), clientIdServer_(NULL) {
	threadPool_ = new ThreadPool(4);
}

MessageDispatcher::MessageDispatcher(
		ServiceRegistrationRequester* registrationRequester,
		ServiceResultServer* resultServer) :
	registrationRequester_(registrationRequester), registrationServer_(NULL),
			permissionRequester_(NULL), permissionServer_(NULL),
			resultRequester_(NULL), resultServer_(resultServer),
			clientIdRequester_(NULL), clientIdServer_(NULL) {
	threadPool_ = new ThreadPool(4);
}

MessageDispatcher::MessageDispatcher(
		ServiceRegistrationServer* registrationServer,
		PermissionServer* permissionServer,
		ServiceResultRequester* resultRequester,
		ServiceResultServer* resultServer, ClientIdServer* clientIdServer) :
	registrationRequester_(NULL), registrationServer_(registrationServer),
			permissionRequester_(NULL), permissionServer_(permissionServer),
			resultRequester_(resultRequester), resultServer_(resultServer),
			clientIdRequester_(NULL), clientIdServer_(clientIdServer) {
	threadPool_ = new ThreadPool(4);
}

MessageDispatcher::~MessageDispatcher() {
	threadPool_->stop_all();
	delete threadPool_;
}

/**
 * Add a new peer node to the available peer list
 */
void MessageDispatcher::addPeerNode(const MessageFrameworkNode& peerNode,
		AsyncStream* bindedStream) {
	std::cout << "Add peer node : " << peerNode.nodeIP_ << ":"
			<< peerNode.nodePort_ << std::endl;
	boost::mutex::scoped_lock lock(availablePeerNodesMutex_);
	availablePeerNodes_.insert(pair<MessageFrameworkNode, AsyncStream*>(
			peerNode, bindedStream));
}

/**
 * Remove a a peer node from the available peer lis
 */
void MessageDispatcher::removePeerNode(const MessageFrameworkNode& peerNode) {
	std::cout << "Remove peer node : " << peerNode.nodeIP_ << ":"
			<< peerNode.nodePort_ << std::endl;
	boost::mutex::scoped_lock lock(availablePeerNodesMutex_);
	availablePeerNodes_.erase(peerNode);
}

bool MessageDispatcher::isExist(const MessageFrameworkNode& node) {
	boost::mutex::scoped_lock lock(availablePeerNodesMutex_);
	std::map<MessageFrameworkNode, AsyncStream*, MessageFrameworkNode_less>::const_iterator
			iter;
	iter = availablePeerNodes_.find(node);
	if (iter != availablePeerNodes_.end()) {
		return true;
	}

	std::cout << "Node: " << node.nodeIP_ << ":" << node.nodePort_
			<< " does not exists." << std::endl;
	return false;
}

AsyncStream& MessageDispatcher::getStreamByNode(const MessageFrameworkNode& node) {
	boost::mutex::scoped_lock lock(availablePeerNodesMutex_);
	std::map<MessageFrameworkNode, AsyncStream*, MessageFrameworkNode_less>::const_iterator
			iter;
	iter = availablePeerNodes_.find(node);
	if (iter == availablePeerNodes_.end()) {
		std::cout << "getStreamByNode fails. Node: " << node.nodeIP_ << ":"
				<< node.nodePort_ << " does not exist." << std::endl;
		throw MessageFrameworkException(SF1_MSGFRK_DATA_NOT_FOUND, __LINE__, __FILE__);
	}

	return *(iter->second);
}

void MessageDispatcher::sendDataToUpperLayer(
		const MessageFrameworkNode& source, MessageType messageType, //const std::string& data)
		boost::shared_ptr<izenelib::util::izene_streambuf>& buffer) {
	if (threadPool_) {
		// don't transfer data by reference,
		// it's a local variable in AsyncStream::handle_read
		ThreadPool::Task task = boost::bind(
				&MessageDispatcher::sendDataToUpperLayer_impl, this,
				boost::cref(source), messageType, buffer);
		threadPool_->executeTask(task);
	} else
		sendDataToUpperLayer_impl(source, messageType, buffer);
}



void MessageDispatcher::sendDataToUpperLayer_impl(
		const MessageFrameworkNode& source, MessageType messageType, //const std::string& data)
		boost::shared_ptr<izenelib::util::izene_streambuf>& buffer) {
	boost::posix_time::ptime before =
			boost::posix_time::microsec_clock::local_time();
#ifdef _LOGGING_
	WriteToLog("log.log", " sendDataToUpperLayer ..." );
#endif
	ServiceRegistrationMessage registrationMsg;
	ServiceRegistrationReplyMessage registrationReplyMessage;
	//PermissionRequestMessage permissionRequestMessage;
	std::string permissionRequestMessage;
	//PermissionOfServiceMessage permissionOfServiceMessage;
	ServicePermissionInfo permissionOfServiceMessage;
	ClientIdRequestMessage clientIdRequestMessage;
	ClientIdReplyMessage clientIdReplyMessage;
	ServiceMessagePtr serviceMessage(new ServiceMessage);
	//boost::archive::binary_iarchive archive(*buffer);

	//cout<<"Debug: MessageDispatcher::sendDataToUpperLayer_impl messageType="
	//		<<messageType<<endl;
	switch (messageType) {
	case SERVICE_REGISTRATION_REQUEST_MSG:
		// forward this message to RegistrationRequester
		//archive >> registrationMsg;
		from_buffer(registrationMsg, buffer);
		registrationServer_->receiveServiceRegistrationRequest(source,
				registrationMsg);
		break;

	case SERVICE_REGISTRATION_REPLY_MSG:
		// forward this message to RegistrationServer
		//archive >> registrationReplyMessage;
		from_buffer(registrationReplyMessage, buffer);
		registrationRequester_->receiveServiceRegistrationReply(
				registrationReplyMessage.getServiceName(),
				registrationReplyMessage.getStatus());
		break;

	case SERVICE_REQUEST_MSG:
		//archive >> *serviceMessage;       
		//cout<<"???? SERVICE_REQUEST_MSG"<<endl;
		from_buffer(*serviceMessage, buffer);
	//	boost::posix_time::ptime after =
	//			boost::posix_time::microsec_clock::local_time();
	//	deserialization_time += (after-before).total_microseconds();
		resultServer_->receiveServiceRequest(source, serviceMessage);
		break;

	case SERVICE_RESULT_MSG:
		//archive >> *serviceMessage;
		//cout<<"???? SERVICE_RESULT_MSG"<<endl;
		from_buffer(*serviceMessage, buffer);
		//from_buffer(serviceMessage, buffer);
	//	boost::posix_time::ptime after =
	//			boost::posix_time::microsec_clock::local_time();
	//	deserialization_time += (after-before).total_microseconds();
		resultRequester_->receiveResultOfService(serviceMessage);
		break;

	case PERMISSION_OF_SERVICE_REQUEST_MSG:
		//archive >> permissionRequestMessage;
		from_buffer(permissionRequestMessage, buffer);
		permissionServer_->receivePermissionOfServiceRequest(source,
				permissionRequestMessage);
		break;

	case PERMISSION_OF_SERVICE_REPLY_MSG:
		//archive >> permissionOfServiceMessage;
		from_buffer(permissionOfServiceMessage, buffer);
		permissionRequester_->receivePermissionOfServiceResult(permissionOfServiceMessage);		
		break;

	case CLIENT_ID_REQUEST_MSG:
		//archive >> clientIdRequestMessage;
		from_buffer(clientIdRequestMessage, buffer);
		clientIdServer_->receiveClientIdRequest(source);
		break;

	case CLIENT_ID_REPLY_MSG:
		//archive >> clientIdReplyMessage;
		from_buffer(clientIdReplyMessage, buffer);
		clientIdRequester_->receiveClientIdResult(clientIdReplyMessage.getClientId());
		break;

	default:
		throw MessageFrameworkException(SF1_MSGFRK_UNKNOWN_DATATYPE, __LINE__, __FILE__);
	}

}

}// end of namespace messageframework

