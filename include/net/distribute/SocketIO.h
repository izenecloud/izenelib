#ifndef SOCKE_IO_H_
#define SOCKE_IO_H_

#include <iostream>
#include <sstream>
#include <string>
#include <sys/time.h>

namespace net{
namespace distribute{

struct ServerAddress
{
    std::string host_;
    unsigned int port_;
};

/**
 * xxx Message header
 * |header|  data  |
 */
struct MessageHeader
{
    const static int FIXED_HEAD_LEN = 256;

    char head[FIXED_HEAD_LEN];

    void addFileName(const std::string& filename);

    const char* getHead()
    {
        return head;
    }

    static int getHeadLen()
    {
        return FIXED_HEAD_LEN;
    }
};

class SocketIO
{
public:
    SocketIO();

    SocketIO(int fd);

    ~SocketIO();

    /// setup a socket that listens for connections
    int Listen(unsigned int port);

    /// accept connection on a socket.
    SocketIO* Accept();

    /// connect to a remote address
    /// @return 0 on success, -1 on failure
    int Connect(const std::string& hostname, unsigned int port);

    int syncSend(const char *buf, int bufLen);

    int syncRecv(char *buf, int bufLen, struct timeval &timeout);


    int Send(const char *buf, int bufLen);

    int Recv(char *buf, int bufLen);

    void Close();

    int getSockFd()
    {
        return sockFd_;
    }

    bool isGood()
    {
        return sockFd_ > 0;
    }

private:
    void SetSocketOpt();

private:
    int sockFd_;
};

}}


#endif /* SOCKE_IO_H_ */
