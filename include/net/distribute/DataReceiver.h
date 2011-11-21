#ifndef DATA_RECEIVER_H_
#define DATA_RECEIVER_H_

#include "SocketIO.h"

namespace net{
namespace distribute{

class DataReceiver
{
    typedef int buf_size_t;
    const static buf_size_t DEFAULT_BUFFER_SIZE = 64*1024; // 64k

public:
    DataReceiver(unsigned int port, buf_size_t bufSize=DEFAULT_BUFFER_SIZE);

    ~DataReceiver();

    void start();

private:
    void doReceive(SocketIO* sock);

private:
    unsigned int port_;

    SocketIO sockIO_;

    char* buf_;
    buf_size_t bufSize_;
};

}}

#endif /* DATA_RECEIVER_H_ */
