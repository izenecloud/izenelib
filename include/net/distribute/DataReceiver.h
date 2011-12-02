#ifndef DATA_RECEIVER_H_
#define DATA_RECEIVER_H_

#include "SocketIO.h"
#include "Msg.h"

#include <fstream>
#include <queue>
#include <boost/thread.hpp>

namespace net{
namespace distribute{

class DataReceiver
{
public:
    typedef int buf_size_t;
    const static buf_size_t DEFAULT_BUFFER_SIZE = 64*1024; // 64k

public:
    DataReceiver(
            unsigned int port,
            const std::string& baseDataDir=".",
            buf_size_t bufSize=DEFAULT_BUFFER_SIZE,
            size_t threadNum=5);

    ~DataReceiver();

    /// accept incoming connections
    void start();

    void stop();

private:
    void enqueue(SocketIO* accSock);
    SocketIO* dequeue();

    void checkDataBaseDir();

    /// receive data on accepted connection
    void receive();

    void doReceive(SocketIO* sock);

    bool receiveHeader(SocketIO* sock, char* buf, buf_size_t bufSize, SendFileReqMsg& fileMsg);

    bool createFile(SendFileReqMsg& fileMsg, std::ofstream& ofs);

    bool receiveData(SocketIO* sock, SendFileReqMsg& fileMsg, std::ofstream& ofs, char* buf, buf_size_t bufSize);

private:
    unsigned int port_;
    std::string baseDataDir_;
    buf_size_t bufSize_;

    SocketIO sockIO_;
    bool isStopping_;

    std::queue<SocketIO*> connQueue_;
    boost::thread_group threadPool_;

    boost::mutex mutex_queue_;
    boost::condition_variable condition_;
};

}}

#endif /* DATA_RECEIVER_H_ */
