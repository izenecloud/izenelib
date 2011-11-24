#include <net/distribute/DataReceiver.h>
#include <net/distribute/SocketIO.h>

#include <string.h>
#include <assert.h>
#include <fstream>

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
        //std::cout<<"--> accept"<<std::endl;
        SocketIO* accSock = sockIO_.Accept();

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
            delete accSock;
            std::cout<<e.what()<<std::endl;
        }
    }
}

void DataReceiver::doReceive(SocketIO* sock)
{
    // thread keeps it's own buffer
    char* buf = new char[bufSize_];
    assert(buf);
    memset(buf, 0, bufSize_);

    struct timeval timeout = {180,0};

    // read head
    int nread=0;
    nread = sock->syncRecv(buf, MessageHeader::getHeadLen(), timeout);
    LOG(INFO)<<"Thrd "<<boost::this_thread::get_id()<<" Fd "<<sock->getSockFd()<<", Received header "<<nread <<" - "<<buf;

    std::string fileName(buf, nread);
    fileName = baseDataDir_+"/"+fileName;
    //std::cout<<fileName<<std::endl;

    // create dir for file to receive
    bfs::path path(fileName);
    std::string parent = path.parent_path().string();
    if (!parent.empty())
        bfs::create_directories(parent);

    // create file
    std::ofstream ofs;
    ofs.open(fileName.c_str());
    if (!ofs.is_open())
    {
        std::cout<<"failed to open : "<<fileName<<std::endl;
    }

    // receive data
    int totalread=0;
    //std::cout <<sock->getSockFd()<< "[DataReceiver] receive data "<<std::endl;
    while ((nread = sock->syncRecv(buf, bufSize_, timeout)) > 0)
    {
        //std::cout <<sock->getSockFd()<< "[DataReceiver] received "<<nread/*<<", \t["<<buf<<"]"*/<<std::endl;
        totalread+=nread;
        ofs.write(buf, nread);
    }
    LOG(INFO)<<"Thrd "<<boost::this_thread::get_id()<<" Fd "<<sock->getSockFd()<<", Received data size "<<totalread;

    // response, xxx check filezie, checksum?

    ofs.close();
    delete sock;
}

}}

