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

    DataTransfer(const std::string& hostname, unsigned int port, buf_size_t bufSize=DEFAULT_BUFFER_SIZE);

    ~DataTransfer();

    /**
     * Synchronously send file or file(s) in directory
     * @param src  full file name or directory name which contain file(s) to be sent.
     * @param curDir  directory name which should contain the file(s) to be sent,
     *                if src is a dir, it will be renamed to curDir.
     * @param isRecursively  whether send recursively, only available when src is a directory.
     * @return
     */
    int syncSend(const std::string& src, const std::string& curDirName, bool isRecursively=false);

    /**
     * Copy file or directory
     * @param src source file or directory name
     * @param dest destination file or directory name
     * @param isRecursively whether send recursively
     * @return
     */
    //int localSend(const std::string& src, const std::string& dest, bool isRecursively=false);

private:
    /**
     * Recursively sent dir
     * @param curDir  full directory path
     * @param parentDir
     * @return
     */
    int syncSendDirRecur(const std::string& curDir, const std::string& parentDir);

    /**
     * Synchronously send file
     * @param fileName  full file name
     * @param curDir  directory to save file (relatively)
     * @return
     */
    int syncSendFile(const std::string& fileName, const std::string& curDir);

    /**
     * Synchronously send data
     * @param buf
     * @param bufLen
     * @return sent length
     */
    //int syncSend(const char* buf, buf_size_t bufLen);

    std::string processPath(const std::string& path);

private:
    SocketIO socketIO_;
    ServerAddress serverAddr_;

    char* buf_;
    buf_size_t bufSize_;
};

}} // namespace

#endif /* DATA_TRANSFER_H_ */
