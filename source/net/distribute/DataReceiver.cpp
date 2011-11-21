#include <net/distribute/DataReceiver.h>
#include <net/distribute/SocketIO.h>

#include <string.h>
#include <assert.h>
#include <fstream>

namespace net{
namespace distribute{

DataReceiver::DataReceiver(unsigned int port, buf_size_t bufSize)
:port_(port), bufSize_(bufSize)
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
        // xxx start a thread
        doReceive(accSock);
    }
}

void DataReceiver::doReceive(SocketIO* sock)
{
    struct timeval timeout = {180,0};

    // read head
    int nread=0;
    nread = sock->syncRecv(buf_, MessageHeader::getHeadLen(), timeout);
    std::cout<<sock->getSockFd()<<"[DataReceiver] received header size "<<nread<<" - "<<buf_<<std::endl;

    std::string fileName(buf_, nread);
    std::cout<<"filename : "<<fileName<<std::endl;
    std::ofstream ofs;
    ofs.open(fileName.c_str());

    int totalread=0;
    //std::cout <<sock->getSockFd()<< "[DataReceiver] receive data "<<std::endl;
    while ((nread = sock->syncRecv(buf_, bufSize_, timeout)) > 0)
    {
        //std::cout <<sock->getSockFd()<< "[DataReceiver] received "<<nread/*<<", \t["<<buf<<"]"*/<<std::endl;
        totalread+=nread;
        ofs.write(buf_, nread);
    }
    std::cout <<sock->getSockFd()<< "[DataReceiver] received data size "<<totalread<<std::endl;

    // xxx check filezie, checksum

    ofs.close();
    delete sock;
}

}}

