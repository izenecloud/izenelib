#include <net/distribute/DataReceiver.h>

#include <string.h>
#include <assert.h>
#include <signal.h>

#include <boost/filesystem.hpp>

#include <glog/logging.h>

namespace bfs = boost::filesystem;

namespace net{
namespace distribute{

DataReceiver::DataReceiver(
        unsigned int port,
        const std::string& baseDataDir,
        buf_size_t bufSize,
        size_t threadNum)
: port_(port)
, baseDataDir_(baseDataDir)
, bufSize_(bufSize)
, isStopping_(false)
{
    if (threadNum < 1) {
        threadNum = 1;
    }

    for (size_t i = 0; i < threadNum; i++)
    {
        threadPool_.create_thread(
                boost::bind(&DataReceiver::receive, this));
    }

    //std::cout<<"--> listen on receiver "<<std::endl;
    sockIO_.Listen(port_);
}

DataReceiver::~DataReceiver()
{
}

void DataReceiver::start()
{
    while (true)
    {
        SocketIO* accSock = sockIO_.Accept();
        std::cout<<"--> accepted"<<std::endl;

        if (accSock != NULL)
        {
            enqueue(accSock);
        }
        else
        {
            if (isStopping_)
            {
                // To exit, we have to unblock accept first (call to stop()),
                //boost::this_thread::interruption_point();
                // we set a stop flag to break immediately instead of interrupt.
                break;
            }
            else
            {
                sockIO_.Close();
                sockIO_.Listen(port_);
            }
        }
    }
}

void DataReceiver::stop()
{
    isStopping_ = true;
    sockIO_.Shutdown(); // unblock recv/send

    threadPool_.interrupt_all();
    threadPool_.join_all();;
}

void DataReceiver::enqueue(SocketIO* accSock)
{
    boost::unique_lock<boost::mutex> lock(mutex_queue_);

    connQueue_.push(accSock);

    condition_.notify_one();
}

SocketIO* DataReceiver::dequeue()
{
    boost::unique_lock<boost::mutex> lock(mutex_queue_);

    if (connQueue_.empty()) {
        condition_.wait(lock); // unlock and wait
    }

    SocketIO* ret = connQueue_.front();
    connQueue_.pop();

    return ret;
}

void DataReceiver::receive()
{
    while (true)
    {
        SocketIO* accSock = dequeue();

        try
        {
            doReceive(accSock);
        }
        catch (std::exception& e)
        {
            std::cout<<e.what()<<std::endl;
        }

        delete accSock;
    }
}

void DataReceiver::doReceive(SocketIO* sock)
{
    // thread keeps it's own buffer
    char* buf = new char[bufSize_];
    assert(buf);
    memset(buf, 0, bufSize_);

    // ignore
    signal(SIGPIPE, SIG_IGN);

    SendFileReqMsg fileMsg;
    if (receiveHeader(sock, buf, bufSize_, fileMsg))
    {
        // received header
        bool readyToReceive = true;
        std::string errorInfo;

        std::ofstream ofs; // xxx overwrite
        if (!createFile(fileMsg, ofs))
        {
            readyToReceive = false;
            errorInfo = "Failed to create file.";
        }

        // response on ready
        ResponseMsg msg;
        msg.setStatus((readyToReceive ? "success" : "failed"));
        msg.setErrorInfo(errorInfo);
        sock->syncSend(msg.toString().c_str(), msg.toString().size());

        // receive data & response
        if (readyToReceive)
        {
            receiveData(sock, fileMsg, ofs,  buf, bufSize_);
        }
    }

    delete[] buf;
}

bool DataReceiver::receiveHeader(SocketIO* sock, char* buf, buf_size_t bufSize, SendFileReqMsg& fileMsg)
{
    struct timeval timeout = {180,0};

    // assume that bufSize is larger than header length
    // xxx use msg end flag
    int nread = sock->syncRecv(buf, bufSize_, timeout);

    if (nread <= 0)
    {
        LOG(ERROR)<<"Failed to receive file info header: "<<nread;
        return false;
    }

    try
    {
        fileMsg.loadMsg(std::string(buf, nread));
    }
    catch(std::exception& e)
    {
        LOG(ERROR)<<"Failed to parse file info header: "<<e.what();
        return false;
    }

    LOG(INFO)<<"Thrd "<<boost::this_thread::get_id()<<" Fd "<<sock->getSockFd()
             <<", Received header "<<nread;//<<" - "<<std::string(buf, nread);
    return true;
}

bool DataReceiver::createFile(SendFileReqMsg& fileMsg, std::ofstream& ofs)
{
    std::string fileName = fileMsg.getFileName(); // relative path
    fileName = baseDataDir_+"/"+fileName;         // to local path
    std::cout<<fileName<<std::endl;

    bfs::path path(fileName);
    std::string parent = path.parent_path().string();
    if (!parent.empty() && !bfs::exists(parent))
    {
        bfs::create_directories(parent);
    }

    ofs.open(fileName.c_str());
    if (!ofs.is_open())
    {
        std::cout<<"failed to open : "<<fileName<<std::endl;
        return false;
    }

    return true;
}

bool DataReceiver::receiveData(SocketIO* sock, SendFileReqMsg& fileMsg, std::ofstream& ofs, char* buf, buf_size_t bufSize)
{
    struct timeval timeout = {180,0};

    size_t fileSize = fileMsg.getFileSize();

    // receive file data
    size_t nrecv, totalRecv=0;
    LOG(INFO)<<"Thrd "<<boost::this_thread::get_id()<<" Fd "<<sock->getSockFd()
             << ", receiving data (size: "<<fileSize<<")";
    int progress, progress_step = 0;
    while ((nrecv = sock->syncRecv(buf, bufSize_, timeout)) > 0)
    {
        totalRecv += nrecv;
        //std::cout <<sock->getSockFd()<< "[DataReceiver] received "<<nrecv/*<<", \t["<<buf<<"]"*/
        //          <<", total "<<totalRecv<<std::endl;
        ofs.write(buf, nrecv);

        if (((progress_step++) % 100) == 0)
        {
            progress = totalRecv*100/fileSize;
            std::cout<<"\r progress "<<progress<<"%"<<std::flush;
        }

        if (totalRecv >= fileSize)
        {
            std::cout<<"\r progress 100%"<<std::flush;
            std::cout<<std::endl;
            break;
        }
    }
    ofs.close();

    LOG(INFO)<<"Thrd "<<boost::this_thread::get_id()<<" Fd "<<sock->getSockFd()
             <<", Received data size "<<totalRecv;

    // response on receive finished
    std::string status = "success";
    std::string errorInfo = "ok";
    if (totalRecv < fileSize)
    {
        status = "failed";
        std::ostringstream oss;
        oss<<"Received incompleted data, received "<<totalRecv<<", excepted "<<fileSize;
        errorInfo = oss.str();
    }

    ResponseMsg msg;
    msg.setStatus(status);
    msg.setErrorInfo(errorInfo);
    msg.setReceivedSize(totalRecv);
    sock->syncSend(msg.toString().c_str(), msg.toString().size()); //xxx

    return true;
}

}}

