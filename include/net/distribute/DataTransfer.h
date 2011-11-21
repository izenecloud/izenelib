#ifndef DATA_TRANSFER_H_
#define DATA_TRANSFER_H_

#include "SocketIO.h"

namespace net{
namespace distribute {

class DataTransfer
{
    typedef int buf_size_t;
    const static buf_size_t DEFAULT_BUFFER_SIZE = 64*1024; // 64k

public:

    DataTransfer(buf_size_t bufSize=DEFAULT_BUFFER_SIZE);

    ~DataTransfer();

    bool connectToServer(const std::string& hostname, unsigned int port);
    bool connectToServer(const ServerAddress& serverAddr);

    int syncSend(const char* buf, buf_size_t bufLen);

    /**
     * Synchronously send data file
     * @param src file name to be sent
     * @return
     */
    int syncSendFile(const std::string& src);

    /**
     * Synchronously send data file(s) in directory
     * @param src directory name to be sent
     * @param isRecursively whether send recursively
     * @return
     */
    int syncSendDir(const std::string& src, bool isRecursively=false);


    /**
     * Copy file or directory
     * @param src source file or directory name
     * @param dest destination file or directory name
     * @param isRecursively whether send recursively
     * @return
     */
    int localSend(const std::string& src, const std::string& dest, bool isRecursively=false);

private:
    SocketIO socketIO_;

    char* buf_;
    buf_size_t bufSize_;
};

}} // namespace

#endif /* DATA_TRANSFER_H_ */
