#ifndef MRLITE_SOCKET_COMMUNICATOR_H_
#define MRLITE_SOCKET_COMMUNICATOR_H_

#include <map>
#include <string>
#include <vector>

#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>
#include <net/mrlite/communicator.h>
#include "epoller.h"
#include "signaling_queue.h"
#include "tcp_socket.h"

namespace net{namespace mrlite{
//-----------------------------------------------------------------------------
// Implement Communicator using TCPSocket:
//-----------------------------------------------------------------------------
class SocketCommunicator : public Communicator
{
public:
    SocketCommunicator() {}
    virtual ~SocketCommunicator() {}

    virtual bool Initialize(bool is_map_worker,
                            int num_map_workers,
                            const std::vector<std::string> &reduce_workers,
                            uint32 map_queue_size /*in bytes*/,
                            uint32 reduce_queue_size /*in bytes*/,
                            int worker_id /*zero-based*/);
    virtual int Send(void *src,
                     int size,
                     int receiver_id /*zero-based*/);
    virtual int Send(const std::string &src,
                     int receiver_id /*zero-based*/);
    virtual int Receive(void *dest, int max_size);
    virtual int Receive(std::string *dest);
    virtual bool Finalize();

private:

    bool is_sender_;
    int num_sender_;
    int num_receiver_;
    int worker_id_;
    uint32 map_queue_size_;
    uint32 reduce_queue_size_;

    std::vector<TCPSocket*> sockets_;
    std::vector<SignalingQueue*> send_buffers_;
    boost::scoped_ptr<SignalingQueue> receive_buffer_;

    // socket_id_ stores the map from active socket to id
    std::map<int /*socket*/, int /*worker id*/> socket_id_;

    boost::scoped_ptr<boost::thread> thread_send_;
    boost::scoped_ptr<boost::thread> thread_receive_;

    bool InitSender(const std::vector<std::string> &reducers);
    bool InitReceiver(const std::string &reducer);
    bool FinalizeSender();
    bool FinalizeReceiver();

    static void SendLoop(SocketCommunicator *pcom);
    static void ReceiveLoop(SocketCommunicator *pcom);

    DISALLOW_COPY_AND_ASSIGN(SocketCommunicator);
};

}}  // namespace mrlite
#endif  // MRLITE_SOCKET_COMMUNICATOR_H_
