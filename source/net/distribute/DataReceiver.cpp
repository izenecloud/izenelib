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

DataReceiver::DataReceiver(unsigned int port, const std::string& dataDir, buf_size_t bufSize)
:port_(port), dataDir_(dataDir), bufSize_(bufSize)
{
    buf_ = new char[bufSize];
    assert(buf_);
    memset(buf_, 0, bufSize);

    std::cout<<"--> listen"<<std::endl;
    sockIO_.Listen(port_);
}

DataReceiver::~DataReceiver()
{
    sockIO_.Close();
}

void DataReceiver::start()
{
    while (true)
    {
        std::cout<<"--> accept"<<std::endl;
        SocketIO* accSock = sockIO_.Accept();

        try
        {
            // new thread?
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
    struct timeval timeout = {180,0};

    // read head
    int nread=0;
    nread = sock->syncRecv(buf_, MessageHeader::getHeadLen(), timeout);
    LOG(INFO)<<"Fd"<<sock->getSockFd()<<", Received header size "<<nread/*<<" - "<<buf_*/;

    std::string fileName(buf_, nread);
    std::cout<<fileName<<std::endl;

    bfs::path path(fileName);
    std::string parent = dataDir_+"/"+path.parent_path().string(); //xxx
    //std::cout<<"parent dirs: "<<parent<<std::endl;
    if (!parent.empty())
        bfs::create_directories(parent);

    std::ofstream ofs;
    ofs.open(fileName.c_str());
    if (!ofs.is_open())
    {
        std::cout<<"failed to open : "<<fileName<<std::endl;
    }

    int totalread=0;
    //std::cout <<sock->getSockFd()<< "[DataReceiver] receive data "<<std::endl;
    while ((nread = sock->syncRecv(buf_, bufSize_, timeout)) > 0)
    {
        //std::cout <<sock->getSockFd()<< "[DataReceiver] received "<<nread/*<<", \t["<<buf<<"]"*/<<std::endl;
        totalread+=nread;
        ofs.write(buf_, nread);
    }
    LOG(INFO)<<"Fd"<<sock->getSockFd()<<", Received data size "<<totalread;

    // xxx check filezie, checksum? response

    ofs.close();
    delete sock;
}

}}

