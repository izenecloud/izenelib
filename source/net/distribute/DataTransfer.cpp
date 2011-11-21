#include <net/distribute/DataTransfer.h>

#include <fstream>
#include <string.h>
#include <boost/filesystem.hpp>

namespace bfs = boost::filesystem;

namespace net{
namespace distribute{

DataTransfer::DataTransfer(buf_size_t bufSize)
: bufSize_(bufSize)
{
    buf_ = new char[bufSize];
    assert(buf_);

    memset(buf_, 0, bufSize);
}

DataTransfer::~DataTransfer()
{
    socketIO_.Close();

    delete[] buf_;
}

bool
DataTransfer::connectToServer(const std::string& hostname, unsigned int port)
{
    return (socketIO_.Connect(hostname, port) == 0);
}

bool
DataTransfer::connectToServer(const ServerAddress& serverAddr)
{
    return connectToServer(serverAddr.host_, serverAddr.port_);
}

int
DataTransfer::syncSend(const char* buf, buf_size_t bufLen)
{
    return socketIO_.syncSend(buf, bufLen);
}

int
DataTransfer::syncSendFile(const std::string& src)
{
    if (bfs::is_directory(src))
    {
        std::cout<<"Is a directory: "<<src<<std::endl;
        return -1;
    }

    std::ifstream ifs;
    ifs.open(src.c_str());
    if (!ifs.is_open())
    {
        std::cout<<"Failed to open: "<<src<<std::endl;
        return -1;
    }

    std::cout<<"[DataTransfer] sending file: "<<src<<std::endl;//xxx

    // send head
    bfs::path path(src);
    MessageHeader head;
    head.addFileName(path.filename());
    int nsend = syncSend(head.getHead(), head.getHeadLen());
    std::cout<<"[DataTransfer] sent header size "<<nsend<<" - "<<head.getHead()<<std::endl;

    // send data
    std::streamsize readLen, sendLen, totalLen = 0;
    ifs.read(buf_, bufSize_);
    while ((readLen = ifs.gcount()) > 0)
    {
        sendLen = syncSend(buf_, readLen);
        if (sendLen != readLen)
        {
            ; // xxx
        }
        //std::cout<<"read "<<readLen<<", sent "<<sendLen<<std::endl;
        totalLen += sendLen;

        ifs.read(buf_, bufSize_);
    }
    std::cout<<"[DataTransfer] sent data size "<<totalLen<<std::endl;

    ifs.close();

    return totalLen;
}

int
DataTransfer::syncSendDir(const std::string& src, bool isRecursively)
{
    return 0;
}

int
DataTransfer::localSend(const std::string& src, const std::string& dest, bool isRecursively)
{
    return 0;
}

}}


