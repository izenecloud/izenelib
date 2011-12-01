#ifndef DATA_TRANSFER_H_
#define DATA_TRANSFER_H_

#include "SocketIO.h"
#include "Msg.h"

#include <fstream>

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
     * @return 0 on success, -n on failure (n indicates the file number of failure).
     */
    int syncSend(const std::string& src, const std::string& curDirName, bool isRecursively=false);

    /**
     * Copy file or directory
     * @param src source file or directory name
     * @param dest destination file or directory name
     * @param isRecursively whether copy recursively if src is directory
     * @return
     */
    static int copy(const std::string& src, const std::string& dest, bool isRecursively=false);

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
     * @return 0 on success, -1 on failure
     */
    int syncSendFile(const std::string& fileName, const std::string& curDir);

    size_t syncSendFileData_(std::ifstream& ifs, size_t fileSize);

    /**
     * utility
     */
    std::string processPath(const std::string& path);

private:
    SocketIO socketIO_;
    ServerAddress serverAddr_;

    char* buf_;
    buf_size_t bufSize_;
};

}} // namespace

#endif /* DATA_TRANSFER_H_ */
