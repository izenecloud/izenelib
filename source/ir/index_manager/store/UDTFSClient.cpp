#include <boost/filesystem.hpp>

#include <ir/index_manager/store/UDTFSClient.h>
#ifndef UDT3
#include <ir/index_manager/store/cc.h>
#endif

#include <string.h>

using namespace boost::filesystem;
using namespace izenelib::ir::indexmanager;

string UDTFSClient::serverIP_ = "";
int UDTFSClient::serverPort_ = 0;
int UDTFSClient::reusePort_ = 0;
UDTFSError UDTFSClient::errorInfo_;

UDTFSClient::UDTFSClient()
{
}

UDTFSClient:: ~UDTFSClient()
{
}

int UDTFSClient::init(const string& server, const int port)
{
    serverIP_ = server;
    serverPort_ = port;

    errorInfo_.init();

#ifndef UDT3
    UDT::startup();
#endif

    return 1;
}

int UDTFSClient::try_connect()
{
    UDTSOCKET socket = UDT::socket(AF_INET, SOCK_STREAM, 0);

    if (UDT::INVALID_SOCK == socket)
        return UDTFSError::E_SOCKET;

    //connect
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serverPort_);
#ifndef WIN32
    inet_pton(AF_INET, serverIP_.c_str(), &serv_addr.sin_addr);
#else
    serv_addr.sin_addr.s_addr = inet_addr(serverIP_.c_str());
#endif
    memset(&(serv_addr.sin_zero), '\0', 8);
    if (UDT::ERROR == UDT::connect(socket, (sockaddr*)&serv_addr, sizeof(serv_addr)))
        return UDTFSError::E_CONNECTION;

    UDTFSMessage req;
    req.setType(UDTFS_CLOSE);

    if (UDT::ERROR == UDT::send(socket, req.pcBuffer_, req.dataLength_, 0))
        return UDTFSError::E_SEND;

    UDT::close(socket);

    return UDTFSError::OK;
}


int UDTFSClient::destroy()
{
    serverIP_ = "";
    serverPort_ = 0;
#ifndef UDT3
    UDT::cleanup();
#endif
    return 1;
}


UDTFile::UDTFile()
{
}

UDTFile::~UDTFile()
{
    close();
}

int UDTFile::open(const char* filepath, const string& mode)
{
    curWritePos_ = 0;
    socket_ = UDT::socket(AF_INET, SOCK_STREAM, 0);

    if (UDT::INVALID_SOCK == socket_)
        return UDTFSError::E_SOCKET;

#ifndef UDT3
    bool reuseaddr = true;
    UDT::setsockopt(socket_, 0, UDT_REUSEADDR, &reuseaddr, sizeof(bool));
    CCCVirtualFactory* pCCFactory = new CCCFactory<CUDPBlast>;
    UDT::setsockopt(socket_, 0, UDT_CC, pCCFactory , sizeof(CCCFactory<CUDPBlast>));
    delete pCCFactory;

    sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(reusePort_);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(my_addr.sin_zero), '\0', 8);

#ifdef WIN32
    int mtu = 1052;
    UDT::setsockopt(socket_, 0, UDT_MSS, &mtu, sizeof(int));
#endif

    if (UDT::bind(socket_, (sockaddr*)&my_addr, sizeof(my_addr)) == UDT::ERROR)
        return UDTFSError::E_BIND;

    int size = sizeof(sockaddr_in);
    UDT::getsockname(socket_, (sockaddr*)&my_addr, &size);
    reusePort_ = ntohs(my_addr.sin_port);
#endif
    //connect
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serverPort_);
#ifndef WIN32
    inet_pton(AF_INET, serverIP_.c_str(), &serv_addr.sin_addr);
#else
    serv_addr.sin_addr.s_addr = inet_addr(serverIP_.c_str());
#endif
    memset(&(serv_addr.sin_zero), '\0', 8);
    if (UDT::ERROR == UDT::connect(socket_, (sockaddr*)&serv_addr, sizeof(serv_addr)))
        return UDTFSError::E_CONNECTION;

    UDTFSMessage msg;
    msg.setType(UDTFS_OPEN);
    filepath_ = filepath;
    path my_path(filepath);
    filename_ = my_path.filename();
    char openmode = 0; 
    if(mode.compare("w+b") == 0)
        openmode = FILEOPEN_MODE_TRUNK;//trunk
    else if(mode.compare("r+") == 0)
        openmode = FILEOPEN_MODE_MODIFY;
    else if(mode.compare("a+") == 0)
        openmode = FILEOPEN_MODE_APPEND;
    else
        return UDTFSError::E_UNKNOWN;

    msg.setData(0, &openmode, 1);
    msg.setData(1, filename_.c_str(), filename_.length() + 1);

    if (UDT::ERROR == UDT::send(socket_, msg.pcBuffer_, msg.dataLength_, 0))
        return UDTFSError::E_SEND;

    return UDTFSError::OK;
}

int UDTFile::send(const char* buf, int size)
{
    int ssize = 0;
    while (ssize < size)
    {
        int ss = UDT::send(socket_, buf + ssize, size - ssize, 0);
        if (UDT::ERROR == ss)
            return UDTFSError::E_SEND;

        ssize += ss;
    }

    return ssize;
}

int UDTFile::recv(char* buf, int size)
{
    int rsize = 0;
    while (rsize < size)
    {
        int rs = UDT::recv(socket_, buf + rsize, size - rsize, 0);
        if (UDT::ERROR == rs)
            return UDTFSError::E_RECEIVE;

        rsize += rs;
    }

    return rsize;
}

int64_t UDTFile::write(const char * buf, const int64_t & size)
{
    UDTFSMessage req;
    req.setType(UDTFS_WRITE);
    req.setData(0, (char *)&curWritePos_, 8);
    req.setData(8, (char *)&size, 8);


    int64_t wsize = -1;

    if (UDT::ERROR == UDT::send(socket_, req.pcBuffer_, req.dataLength_, 0))
        return UDTFSError::E_SEND;

    wsize = send(buf, size);

    if (wsize > 0)
        curWritePos_ += wsize;

    return wsize;
}


int UDTFile::upload()
{
    UDTFSMessage req;
    req.setType(UDTFS_UPLOAD);
#ifndef UDT3
    fstream ifs;
#else
    ifstream ifs;
#endif
    ifs.open(filepath_.c_str(), ios::in | ios::binary);
    if (ifs.fail() || ifs.bad())
        return UDTFSError::E_UNKNOWN;

    ifs.seekg(0, ios::end);
    int64_t size = ifs.tellg();
    ifs.seekg(0);

    req.setData(0, (char *)&size, 8);
    if (UDT::ERROR == UDT::send(socket_, req.pcBuffer_, req.dataLength_, 0))
        return UDTFSError::E_SEND;
    if (UDT::ERROR == UDT::sendfile(socket_, ifs, 0, size))
        return UDTFSError::E_SEND;

    return UDTFSError::OK;
}


int UDTFile::seek(int64_t pos)
{
    curWritePos_+=pos;
    return 1;
}

int UDTFile::close()
{
    UDTFSMessage req;
    req.setType(UDTFS_CLOSE);

    if (UDT::ERROR == UDT::send(socket_, req.pcBuffer_, req.dataLength_, 0))
        return UDTFSError::E_SEND;

    UDT::close(socket_);
    return 1;
}


