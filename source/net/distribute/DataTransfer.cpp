#include <net/distribute/DataTransfer.h>

#include <fstream>
#include <string.h>
#include <signal.h>
#include <boost/filesystem.hpp>

#include <glog/logging.h>

namespace bfs = boost::filesystem;

namespace net{
namespace distribute{

DataTransfer::DataTransfer(const std::string& hostname, unsigned int port, buf_size_t bufSize)
: serverAddr_(hostname, port), bufSize_(bufSize)
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

int
DataTransfer::syncSend(const std::string& src, const std::string& curDirName, bool isRecursively)
{
    if (!bfs::exists(src))
    {
        std::cout<<"Not exists: "<<src<<std::endl;
        return -1;
    }

    // src is a file
    if (!bfs::is_directory(src))
    {
        bfs::path path(src);
        std::string curFileDir = curDirName.empty() ? path.parent_path().filename() : curDirName;
        return syncSendFile(src, curFileDir);
    }

    // src is a directory
    bfs::path path(processPath(src));
    //std::cout<<path.string()<<std::endl;
    std::string curFileDir = curDirName.empty() ? path.filename() : curDirName; // rename dir to dirName
    //std::cout<<"[DataTransfer] dir: "<<curFileDir<<std::endl;

    int ret = 0;
    int sent = 0;
    bfs::directory_iterator iterEnd;
    for (bfs::directory_iterator iter(path); iter != iterEnd; iter++)
    {
        if (bfs::is_regular_file(iter->path()))
        {
            //std::cout<<iter->path().filename()<<std::endl;
            sent = syncSendFile(iter->path().string(), curFileDir);
            if (sent > 0)
                ret += sent;
        }
        else if (isRecursively && bfs::is_directory(iter->path()))
        {
            std::string subDir = iter->path().string();
            sent = syncSendDirRecur(subDir, curFileDir);
            if (sent > 0)
                ret += sent;
        }
    }

    return ret;
}

int
DataTransfer::copy(const std::string& src, const std::string& dest, bool isRecursively)
{
    //bfs::copy_file();
    return 0;
}

/// private ////////////////////////////////////////////////////////////////////
int
DataTransfer::syncSendDirRecur(const std::string& curDir, const std::string& parentDir)
{
    bfs::path path(curDir);
    std::string curFileDir = parentDir + "/" + path.filename();
    //std::cout<<"[DataTransfer] dir: "<<curFileDir<<std::endl; //xxx

    int ret = 0;
    int sent = 0;
    bfs::directory_iterator iterEnd;
    for (bfs::directory_iterator iter(curDir); iter != iterEnd; iter++)
    {
        if (bfs::is_regular_file(iter->path()))
        {
            std::cout<<iter->path().filename()<<std::endl;//xxx
            sent = syncSendFile(iter->path().string(), curFileDir);
            if (sent > 0)
                ret += sent;
        }
        else if (bfs::is_directory(iter->path()))
        {
            std::string subDir = iter->path().string();
            sent = syncSendDirRecur(subDir, curFileDir);
            if (sent > 0)
                ret += sent;
        }
    }

    return ret;
}

int
DataTransfer::syncSendFile(const std::string& fileName, const std::string& curDir)
{
    if (bfs::is_directory(fileName))
    {
        std::cout<<"Is a directory: "<<fileName<<std::endl;
        return -1;
    }

    std::ifstream ifs;
    ifs.open(fileName.c_str());
    if (!ifs.is_open())
    {
        std::cout<<"Failed to open: "<<fileName<<std::endl;
        return -1;
    }

    LOG(INFO)<<"Transferring "<<fileName<<" to remote dir "<<curDir;

    // new connection to simplify data control
    if (socketIO_.Connect(serverAddr_.host_, serverAddr_.port_) < 0)
        return -1;

    // don't terminate when server broken, xxx check error
    signal(SIGPIPE, SIG_IGN);

    // send head
    bfs::path path(fileName);
    MessageHeader head;
    head.addFileName(curDir+"/"+path.filename());

    int nsend = socketIO_.syncSend(head.getHead(), head.getHeadLen());
    if (nsend < head.getHeadLen())
        LOG(INFO)<<"Sent header size "<<nsend<<" - "<<head.getHead();

    // send data
    std::streamsize readLen, sendLen, totalLen = 0;

    ifs.read(buf_, bufSize_);
    while ((readLen = ifs.gcount()) > 0)
    {
        sendLen = socketIO_.syncSend(buf_, readLen);
        if (sendLen != readLen)
        {
            ; // xxx
        }
        //std::cout<<"read "<<readLen<<", sent "<<sendLen<<std::endl;
        totalLen += sendLen;

        ifs.read(buf_, bufSize_);
    }

    ifs.close();
    LOG(INFO)<<"Sent data size "<<totalLen;

    // xxx check server receiver status

    return totalLen;
}

//int
//DataTransfer::syncSend(const char* buf, buf_size_t bufLen)
//{
//    return socketIO_.syncSend(buf, bufLen);
//}

std::string DataTransfer::processPath(const std::string& path)
{
    std::string ret = path;
    size_t n = ret.size();

    // remove tailing '/'
    if (n > 0 && ret[n-1] == '/')
    {
        ret.erase(n-1, n);
    }

    return ret;
}

}} // namespace


