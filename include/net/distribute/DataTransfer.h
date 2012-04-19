#ifndef DATA_TRANSFER_H_
#define DATA_TRANSFER_H_

#include "SocketIO.h"
#include "Msg.h"

#include <fstream>

#include <boost/filesystem.hpp>
namespace bfs = boost::filesystem;

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
     * Synchronously send file or directory to remote host
     * @param src  full file name or directory name which contain file(s) to be sent.
     * @param destDir  destination directory in remote host (relative to receiver base dir),
     *                 default is current dir name of src.
     * @param isRecursively  whether send recursively, only available when src is a directory.
     * @return 0 on success, -n on failure.
     */
    int syncSend(const std::string& src, const std::string& destDir, bool isRecursively=false);

    /**
     * Copy file or directory to local
     * @param src source file or directory name
     * @param dest destination file or directory, directory path should be exited.
     * @param isRecursively whether copy recursively if src is directory
     * @return
     */
    static bool copy(const std::string& src, const std::string& dest, bool isRecursively=false, bool isOverwrite=false);

    /**
     * A simple way to move (copy) file or dir
     */
    static bool rename(const std::string& src, const std::string& dest);

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

    int64_t syncSendFileData_(std::ifstream& ifs, const std::string& fileName, int64_t fileSize);

    /**
     * File copy
     */
    static bool copyFile_(const bfs::path& src, const bfs::path& dest, bool isOverwrite);

    /**
     * utility
     */
    static std::string processPath(const std::string& path);

private:
    SocketIO socketIO_;
    ServerAddress serverAddr_;

    char* buf_;
    buf_size_t bufSize_;

    int sentFileNum_;
};

}} // namespace

#endif /* DATA_TRANSFER_H_ */
