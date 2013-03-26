#ifndef MRLITE_EPOLLER_H_
#define MRLITE_EPOLLER_H_

#include <sys/epoll.h>

#include <net/mrlite/common.h>

namespace net{namespace mrlite{

// a simple wrapper of epoll
class Epoller
{
public:
    explicit Epoller(int epoll_size);
    ~Epoller();

    // Close the control file descriptor of the epoll object
    void Close();

    // Return the file descriptor number of the control fd
    int FileNo() const;

    // Register a socket descriptor with the epoll object
    // eventmask:
    //  EPOLLIN, EPOLLOUT, EPOLLRDHUP, EPOLLPRI, EPOLLERR,
    //  EPOLLHUP, EPOLLET, EPOLLONESHOT
    bool Register(int socket, uint32 eventmask);

    // Modify a register file descriptor
    bool Modify(int socket, uint32 eventmask);

    // Remove a registered file descriptor from the epoll object
    bool Unregister(int socket);

    // Wait for events. timeout in milliseconds.
    int Poll(struct epoll_event *events, int max_events, int timeout=-1);

private:
    int epfd_;

    // A wrapper of epoll_ctl(), implement basic operations on the epoll object
    bool Control(int op, int socket, uint32 eventmask);
};

}}  // namespace mrlite

#endif  // MRLITE_EPOLLER_H_
