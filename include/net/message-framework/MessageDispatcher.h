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

namespace messageframework
{
    class MessageClientFull;
    class MessageControllerFull;
    class MessageServerFull;


    extern void sendDataToUpperLayer_impl( MessageServerFull& server, const MessageFrameworkNode& source,
                MessageType messageType, boost::shared_ptr<izenelib::util::izene_streambuf>& buffer);
    extern void sendDataToUpperLayer_impl( MessageClientFull& client, const MessageFrameworkNode& source,
                MessageType messageType, boost::shared_ptr<izenelib::util::izene_streambuf>& buffer);
    extern void sendDataToUpperLayer_impl( MessageControllerFull& controller, const MessageFrameworkNode& source,
                MessageType messageType, boost::shared_ptr<izenelib::util::izene_streambuf>& buffer);

    /**
     * @brief This class plays an intermediary role between network layer
     * and application layer. It converts raw data sent by network layer
     * into object data, and then, sends the object to application layer.
     * It also converts object data sent by application layer into raw data,
     * and then, sends the raw data to newtwork layer.
     **/
    template<typename ApplicationType>
    class MessageDispatcher
    {
        typedef MessageDispatcher<ApplicationType> ThisType;
        typedef AsyncStream<ApplicationType, ThisType> AsyncStreamType;

    public:

        MessageDispatcher(ApplicationType& app) : app_(app) {};

        /**
         * @brief default destructor,
         */
        ~MessageDispatcher() {}

        /**
         * Add a new peer node to the available peer list
         */
        void addPeerNode(const MessageFrameworkNode& peerNode,
                         AsyncStreamType* bindedStream)
        {
            DLOG(INFO) << "Add peer node : " << peerNode.nodeIP_ << ":"
            << peerNode.nodePort_ << std::endl;

            boost::mutex::scoped_lock lock(availablePeerNodesMutex_);
            availablePeerNodes_.insert(pair<MessageFrameworkNode, AsyncStreamType*>(
                peerNode, bindedStream));
        }

        /**
         * Remove a a peer node from the available peer lis
         */
        void removePeerNode(const MessageFrameworkNode& peerNode)
        {
            DLOG(INFO) << "Remove peer node : " << peerNode.nodeIP_ << ":"
            << peerNode.nodePort_ << std::endl;

            boost::mutex::scoped_lock lock(availablePeerNodesMutex_);
            availablePeerNodes_.erase(peerNode);
        }

        bool isExist(const MessageFrameworkNode& node)
        {
            boost::mutex::scoped_lock lock(availablePeerNodesMutex_);
            typename std::map<MessageFrameworkNode, AsyncStreamType*,
                MessageFrameworkNode_less>::const_iterator iter =
                    availablePeerNodes_.find(node);
            if (iter != availablePeerNodes_.end())
            {
                return true;
            }
            DLOG(INFO) << "Node: " << node.nodeIP_ << ":" << node.nodePort_
            << " does not exists." << std::endl;
            return false;
        }

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
                                  boost::shared_ptr<izenelib::util::izene_streambuf>& buffer)
        {
            sendDataToUpperLayer_impl(source, messageType, buffer);
        }

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
            const MessageFrameworkNode& destination)
        {
            return sendDataToLowerLayer_impl<DataType>(messageType, data,destination);
        }

    private:

        /**
         * @brief Retrieve a stream that is binded to a node
         * @param node node where the stream is binded to
         * @return the stream
         * @return NULL if the stream does not exist
         */
        AsyncStreamType& getStreamByNode(const MessageFrameworkNode& node)
        {
            boost::mutex::scoped_lock lock(availablePeerNodesMutex_);
            typename std::map<MessageFrameworkNode, AsyncStreamType*,
                MessageFrameworkNode_less>::const_iterator iter =
                    availablePeerNodes_.find(node);
            if (iter == availablePeerNodes_.end())
            {
                DLOG(ERROR) << "getStreamByNode fails. Node: " << node.nodeIP_ << ":"
                << node.nodePort_ << " does not exist." << std::endl;
                throw MessageFrameworkException(SF1_MSGFRK_DATA_NOT_FOUND, __LINE__, __FILE__);
            }

            return *(iter->second);
        }

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
            boost::shared_ptr<izenelib::util::izene_streambuf> archive_stream(
                new izenelib::util::izene_streambuf());

            to_buffer(data, archive_stream);

            // retrieve stream that is binded to destination
            AsyncStreamType& stream = getStreamByNode(destination);

            // forward data to network layer, the data will be sent
            // to destination
            stream.sendMessage(messageType, archive_stream);

            return true;
        }

        /**
         * This function should be called by threads in threadPool_ or called
         * directly from sendDataToUpperLayer. To avoid arguments be copied when the
         * funtion object generated by boost::bind copied elsewhere, I change all
         * arguments to be pointer.
         */
        void sendDataToUpperLayer_impl( const MessageFrameworkNode& source,
                MessageType messageType, boost::shared_ptr<izenelib::util::izene_streambuf>& buffer)
        {
            messageframework::sendDataToUpperLayer_impl(app_, source, messageType, buffer);
        }

    private:

        ApplicationType& app_;

        /**
         * @brief list of available peer nodes. There is a AsyncStream
         * binded to each peer node
         **/
        std::map<MessageFrameworkNode, AsyncStreamType*, MessageFrameworkNode_less> availablePeerNodes_;
        boost::mutex availablePeerNodesMutex_;

    };


}// end of namespace messageframework

#endif


