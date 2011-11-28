#include <net/distribute/SocketIO.h>

#include <cstdio>
#include <cerrno>
#include <string.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>


namespace net{
namespace distribute{


void MessageHeader::addFileName(const std::string& filename)
{
    memset(head, 0, FIXED_HEAD_LEN);
    strncpy(head, filename.c_str(), filename.size());
}

SocketIO::SocketIO()
:sockFd_(-1)
{
}

SocketIO::SocketIO(int fd)
:sockFd_(fd)
{
}

SocketIO::~SocketIO()
{
    Close();
}

int SocketIO::Listen(unsigned int port)
{
    struct sockaddr_in sa;

    sockFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFd_ == -1) {
        perror("Socket: ");
        return -1;
    }

    memset(&sa, 0, sizeof(struct sockaddr_in));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    sa.sin_port = htons(port);

    int reuseAddr = 1;
    if (setsockopt(sockFd_, SOL_SOCKET, SO_REUSEADDR,
                   (char *) &reuseAddr, sizeof(reuseAddr)) < 0) {
        perror("Setsockopt: ");
    }

    if (bind(sockFd_, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
        perror("Bind: ");
        close(sockFd_);
        sockFd_ = -1;
        return -1;
    }

    if (listen(sockFd_, 1024) < 0) {
        perror("listen: ");
    }

    return 0;
}

SocketIO* SocketIO::Accept()
{
    int fd;
    struct sockaddr_in cliAddr;
    socklen_t cliAddrLen = sizeof(cliAddr);

    if ((fd = accept(sockFd_, (struct sockaddr *) &cliAddr, &cliAddrLen)) < 0) {
        perror("Accept: ");
        return NULL;
    }

    SocketIO *accSock;
    accSock = new SocketIO(fd);
    accSock->SetSocketOpt();

    return accSock;
}

int SocketIO::Connect(const std::string& hostname, unsigned int port)
{
    struct sockaddr_in sa = { 0 };

    // set socket address
    if (! inet_aton(hostname.c_str(), &sa.sin_addr)) {
        // try to get dotted address if failed to convert to Internet address
        struct hostent * const hostInfo = gethostbyname(hostname.c_str());
        if (hostInfo == NULL || hostInfo->h_addrtype != AF_INET ||
                hostInfo->h_length < (int)sizeof(sa.sin_addr))
        {
            std::cerr<<"Failed to connect: "<<hostname<<std::endl;
            return -1;
        }
        memcpy(&sa.sin_addr, hostInfo->h_addr, sizeof(sa.sin_addr));
    }
    sa.sin_port = htons(port);
    sa.sin_family = AF_INET;

    // close
    Close();

    // setup socket collection
    sockFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFd_ < 0)
        return -1;

    int ret;
    ret = connect(sockFd_, (struct sockaddr *)&sa, sizeof(struct sockaddr_in));
    if (ret < 0)
    {
        perror("Connect: ");
        close(sockFd_);
        sockFd_ = -1;
        return -1;
    }

    SetSocketOpt();

    return ret;
}

int SocketIO::syncSend(const char *buf, int bufLen)
{
    int numSent = 0;
    int res = 0, nfds;
    struct pollfd pfd;
    // 1 second
    const int kTimeout = 1000;

    while (numSent < bufLen) {
        if (sockFd_ < 0)
            break;
        if (res < 0) {
            pfd.fd = sockFd_;
            pfd.events = POLLOUT;
            pfd.revents = 0;
            nfds = poll(&pfd, 1, kTimeout);
            if (nfds == 0)
                continue;
        }

        res = Send(buf + numSent, bufLen - numSent);
        if (res == 0)
            return 0;
        if ((res < 0) &&
            ((errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINTR)))
            continue;
        if (res < 0)
            break;
        numSent += res;
        res = -1;
    }

    return numSent;
}

int SocketIO::syncRecv(char *buf, int bufLen, struct timeval &timeout, bool fillUpBuff)
{
    int numRecd = 0;
    int res = 0, nfds;
    struct pollfd pfd;
    struct timeval startTime, now;

    gettimeofday(&startTime, NULL);

    while (numRecd < bufLen) {
        if (sockFd_ < 0)
            break;

        if (res < 0) {
            pfd.fd = sockFd_;
            pfd.events = POLLIN;
            pfd.revents = 0;
            nfds = poll(&pfd, 1, timeout.tv_sec * 1000);
            // get a 0 when timeout expires
            if (nfds == 0) {
                std::cout << "Timeout in synch recv" << std::endl;
                return numRecd > 0 ? numRecd : -ETIMEDOUT;
            }
        }

        gettimeofday(&now, NULL);
        if (now.tv_sec - startTime.tv_sec >= timeout.tv_sec) {
            return numRecd > 0 ? numRecd : -ETIMEDOUT;
        }

        res = Recv(buf + numRecd, bufLen - numRecd);

        if (res == 0)
            break; //return 0;
        if ((res < 0) &&
            ((errno == EAGAIN) || (errno == EWOULDBLOCK) || (errno == EINTR)))
            continue;
        if (res < 0)
            break;
        numRecd += res;

        if (!fillUpBuff)
        {
            // return once received data,
            // not wait for buffer to be filled up
            break;
        }
    }

    return numRecd;
}

int SocketIO::Send(const char *buf, int bufLen)
{
    int nwrote;
    nwrote = bufLen > 0 ? send(sockFd_, buf, bufLen, 0) : 0;

    return nwrote;
}

int SocketIO::Recv(char *buf, int bufLen)
{
    int nread;
    nread = bufLen > 0 ? recv(sockFd_, buf, bufLen, 0) : 0;

    return nread;
}

void SocketIO::Close()
{
    if (sockFd_ < 0) {
        return;
    }
    close(sockFd_);
    sockFd_ = -1;
}

void SocketIO::Shutdown()
{
    shutdown(sockFd_, SHUT_RDWR);

    Close();
}

void SocketIO::SetSocketOpt()
{
    int bufSize = 65536; // xxx 64k

    if (setsockopt(sockFd_, SOL_SOCKET, SO_SNDBUF, (char *) &bufSize, sizeof(bufSize)) < 0) {
        perror("Setsockopt: ");
    }
    if (setsockopt(sockFd_, SOL_SOCKET, SO_RCVBUF, (char *) &bufSize, sizeof(bufSize)) < 0) {
        perror("Setsockopt: ");
    }

    int flag = 1;
    if (setsockopt(sockFd_, SOL_SOCKET, SO_KEEPALIVE, (char *) &flag, sizeof(flag)) < 0) {
        perror("Disabling NAGLE: ");
    }

    fcntl(sockFd_, F_SETFL, O_NONBLOCK);
    // turn off NAGLE
    if (setsockopt(sockFd_, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(flag)) < 0) {
        perror("Disabling NAGLE: ");
    }
}

}}
