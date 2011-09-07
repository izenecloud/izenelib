#include "epoller.h"

namespace net{namespace mrlite{

Epoller::Epoller(int epoll_size)
{
    epfd_ = epoll_create(epoll_size);
    CHECK_LE(0, epfd_);
}

Epoller::~Epoller()
{
    Close();
}

void Epoller::Close()
{
    if (epfd_ >= 0)
    {
        CHECK_EQ(0, close(epfd_));
        epfd_ = -1;
    }
}

int Epoller::FileNo() const
{
    return epfd_;
}

bool Epoller::Register(int fd, uint32 eventmask)
{
    return Control(EPOLL_CTL_ADD, fd, eventmask);
}

bool Epoller::Modify(int fd, uint32 eventmask)
{
    return Control(EPOLL_CTL_MOD, fd, eventmask);
}

bool Epoller::Unregister(int fd)
{
    return Control(EPOLL_CTL_DEL, fd, 0);
}

int Epoller::Poll(struct epoll_event *events, int max_events, int timeout)
{
    return epoll_wait(epfd_, events, max_events, timeout);
}

bool Epoller::Control(int op, int fd, uint32 eventmask)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = eventmask;
    return 0 == epoll_ctl(epfd_, op, fd, &ev);
}

}}  // namespace mrlite
