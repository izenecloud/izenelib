#include <net/distribute/DataTransfer.h>

#include <string.h>
#include <signal.h>

#include <glog/logging.h>

namespace bfs = boost::filesystem;

namespace net{
namespace distribute{

DataTransfer::DataTransfer(const std::string& hostname, unsigned int port, buf_size_t bufSize)
: serverAddr_(hostname, port), bufSize_(bufSize), sentFileNum_(0)
{
    buf_ = new char[bufSize];
    assert(buf_);
    memset(buf_, 0, bufSize);

    //socketIO_.Connect(hostname, port);
}

DataTransfer::~DataTransfer()
{
    socketIO_.Close();

    delete[] buf_;
}

int
DataTransfer::syncSend(const std::string& src, const std::string& destDir, bool isRecursively)
{
    if (!bfs::exists(src))
    {
        LOG(ERROR)<<"Path does not exists: "<<src<<std::endl;
        return -1;
    }

    // src is a file
    if (!bfs::is_directory(src))
    {
        bfs::path path(src);
        std::string curFileDir = destDir.empty() ? path.parent_path().filename() : destDir;
        return syncSendFile(src, curFileDir);
    }

    // src is a directory
    bfs::path path(processPath(src));
    //std::cout<<path.string()<<std::endl;
    std::string curFileDir = destDir.empty() ? path.filename() : destDir; // rename dir to dirName
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
            ret += sent;
        }
        else if (isRecursively && bfs::is_directory(iter->path()))
        {
            std::string subDir = iter->path().string();
            sent = syncSendDirRecur(subDir, curFileDir);
            ret += sent;
        }
    }

    if (sentFileNum_ == 0)
    {
        LOG(ERROR)<<"Dir is empty.";
        return -1;
    }

    return ret;
}

/*static*/ bool
DataTransfer::copy(const std::string& src, const std::string& dest, bool isRecursively, bool isOverwrite)
{
    if (!bfs::exists(src))
    {
        LOG(ERROR)<<"No such file or directory: "<<src<<std::endl;
        return false;
    }

    processPath(dest);

    // src is a file
    if (!bfs::is_directory(src))
    {
        if (bfs::is_regular_file(src))
        {
            bfs::path srcPath(src);
            bfs::path destPath(dest);
            if (bfs::is_directory(destPath))
            {
                destPath = destPath / srcPath.filename();
            }
            return copyFile_(srcPath, destPath, isOverwrite);
        }
    }
    // src is a directory
    else
    {
        if (!bfs::is_directory(dest))
        {
            LOG(ERROR)<<"Destination is not a directory or not existed: "<<dest<<std::endl;
            return false;
        }

        bfs::directory_iterator iterEnd;
        for (bfs::directory_iterator iter(src); iter != iterEnd; iter++)
        {
            if (bfs::is_regular_file(iter->path()))
            {
                if (!copyFile_(iter->path(), bfs::path(dest) / iter->path().filename(), isOverwrite))
                    return false;
            }
            else if (isRecursively && bfs::is_directory(iter->path()))
            {
                std::string recDir = dest+"/"+iter->path().filename();
                bfs::create_directories(recDir);
                copy(iter->path().string(), recDir, isRecursively, isOverwrite);
            }
        }
    }

    return true;
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
    sentFileNum_ = 0;
    bfs::directory_iterator iterEnd;
    for (bfs::directory_iterator iter(curDir); iter != iterEnd; iter++)
    {
        if (bfs::is_regular_file(iter->path()))
        {
            //std::cout<<iter->path().filename()<<std::endl;//xxx
            sent = syncSendFile(iter->path().string(), curFileDir);
            ret += sent;
        }
        else if (bfs::is_directory(iter->path()))
        {
            std::string subDir = iter->path().string();
            sent = syncSendDirRecur(subDir, curFileDir);
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

    sentFileNum_ ++;
    //LOG(INFO)<<"Transferring "<<fileName<<" to remote dir "<<curDir;

    /// start a new connection to send file, adjust Receiver if keep one connection
    socketIO_.Connect(serverAddr_.host_, serverAddr_.port_);
    if (!socketIO_.isGood())
    {
        LOG(ERROR)<<"socket error";
        return -1;
    }

    // don't terminate when server broken, xxx check error
    signal(SIGPIPE, SIG_IGN);

    struct timeval timeout = {180,0};

    /// Send head info
    SendFileReqMsg msg;
    bfs::path path(fileName);
    msg.setFileType(SendFileReqMsg::FTYPE_SCD);
    msg.setFileName(curDir+"/"+path.filename());
    int64_t fileSize = bfs::file_size(fileName);
    msg.setFileSize(fileSize);

    std::string msg_head = msg.toString();
    int nsend = socketIO_.syncSend(msg_head.c_str(), msg_head.size());
    //LOG(INFO)<<"Sent header size "<<nsend<<" - "<<msg_head;

    if (nsend < msg_head.size())
    {
        LOG(ERROR)<<"Failed to send file header info";
        return -1;
    }

    /// Check Receiver status: ready to receive?
    int nrecv;
    ResponseMsg resMsg;
    nrecv = socketIO_.syncRecv(buf_, bufSize_, timeout);
    resMsg.loadMsg(std::string(buf_, nrecv));
    //LOG(INFO)<<"Ready? "<<std::string(buf_, nrecv);
    if (nrecv >0 && resMsg.getStatus() != "success")
    {
        LOG(ERROR)<<"Receiver not ready";
        return -1;
    }

    /// Send file data
    int64_t sentDataSize = syncSendFileData_(ifs, path.string(), fileSize);
    if (sentDataSize < fileSize)
    {
        LOG(ERROR)<<"Sent incompleted file data, sent "
                  <<sentDataSize<<", file size "<<fileSize;
        return -1;
    }

    /// Check Receiver status: receive completed?
    nrecv = socketIO_.syncRecv(buf_, bufSize_, timeout);
    if (nrecv > 0)
    {
        resMsg.loadMsg(std::string(buf_, nrecv));
        //LOG(INFO)<<"receiver status? "<<std::string(buf_, nrecv);
        // error info?
        size_t nrecvedDataSize = resMsg.getReceivedSize();

        if (nrecvedDataSize < sentDataSize)
        {
            // xxx, retry?
            LOG(ERROR)<<"Receiver received incompleted data, received"
                      <<nrecvedDataSize<<", total"<<sentDataSize;
            return -1;
        }

        // transfer succeeded
        return 0;
    }
    else
    {
        LOG(ERROR)<<"Failed to get Receiver status";
        return -1;
    }
}

int64_t DataTransfer::syncSendFileData_(std::ifstream& ifs, const std::string& fileName, int64_t fileSize)
{
    int64_t readLen, sentLen, totalSentLen = 0;
    int progress, progress_step = 0;

    ifs.read(buf_, bufSize_);
    while ((readLen = ifs.gcount()) > 0)
    {
        if ((sentLen = socketIO_.syncSend(buf_, readLen)) < readLen)
        {
            ifs.close();
            LOG(ERROR)<<"Error: data buf was sent incompletely, errno="<<errno;
            break;
        }
        totalSentLen += sentLen;

        if (((progress_step++) % 10) == 0)
        {
            progress = totalSentLen*100/fileSize;
            std::cout<<"\r"<<progress<<"%\t"<<fileSize<<"B ("<<MsgHead::size2String(fileSize)<<")\t"<<fileName<<std::flush;
        }

        ifs.read(buf_, bufSize_);
    }
    ifs.close();

    if (totalSentLen >= fileSize)
    {
        std::cout<<"\r100%\t"<<fileSize<<"B ("<<MsgHead::size2String(fileSize)<<")\t"<<fileName<<std::flush;
    }
    else
        std::cout<<"\nincompleted! sent "<<totalSentLen<<", file size "<<fileSize;
    std::cout<<std::endl;

    //LOG(INFO)<<"Sent data size "<<totalSentLen;

    return totalSentLen;
}

/*static*/ bool
DataTransfer::copyFile_(const bfs::path& src, const bfs::path& dest, bool isOverwrite)
{
    //std::cout<<src<<"  --->  "<<dest<<std::endl;
    try
    {
        BOOST_SCOPED_ENUM(bfs::copy_option) option =
            isOverwrite ? bfs::copy_option::overwrite_if_exists : bfs::copy_option::fail_if_exists;
        bfs::copy_file(src, dest, option);
    }
    catch (std::exception& e)
    {
        std::cout<<e.what()<<std::endl;
        return false;
    }

    return true;
}

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


