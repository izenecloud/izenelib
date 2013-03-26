#ifndef MRLITE_TCP_SOCKET_H_
#define MRLITE_TCP_SOCKET_H_

#include <sys/socket.h>
#include <string>

#include <net/mrlite/common.h>

namespace net{namespace mrlite{

// TCPSocket is a simple wrapper around a socket. It supports
// only TCP connections.
class TCPSocket
{
public:
    // ctor and dctor
    TCPSocket();
    ~TCPSocket();

    // Return value of following functions:
    //  true for success and false for failure
    // connect to a given server address
    bool Connect(const char * ip, uint16 port);

    // bind on the given IP ans PORT
    bool Bind(const char * ip, uint16 port);

    // listen
    bool Listen(int max_connection);

    // wait for a new connection
    // new SOCKET, IP and PORT will be stored to socket, ip_client and port_client
    bool Accept(TCPSocket * socket,
                std::string * ip_client,
                uint16 * port_client);

    // SetBlocking() is needed refering to this example of epoll:
    // http://www.kernel.org/doc/man-pages/online/pages/man4/epoll.4.html
    bool SetBlocking(bool flag);

    // Shut down one or both halves of the connection.
    // If ways is SHUT_RD, further receives are disallowed.
    // If ways is SHUT_WR, further sends are disallowed.
    // If ways is SHUT_RDWR, further sends and receives are disallowed.
    bool ShutDown(int ways);

    // close socket
    void Close();

    // send/receive data:
    // return number of bytes read or written if OK, -1 on error
    // caller is responsible for checking that all data has been sent/received,
    // if not, extra send/receive should be invoked
    int Send(const char * data, int len_data);
    int Receive(char * buffer, int size_buffer);

    // return socket's file descriptor
    int Socket() const;

private:
    int socket_;

    DISALLOW_COPY_AND_ASSIGN(TCPSocket);
};

}}  // namespace mrlite

#endif  // MRLITE_TCP_SOCKET_H_

