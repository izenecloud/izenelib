#ifndef __febird_rpc_reactor_h__
#define __febird_rpc_reactor_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include "../refcount.h"
#include <boost/intrusive_ptr.hpp>
#include <boost/coro>

namespace febird {

class ConnectionState : public RefCounter
{
public:
	int fd;
	void* coro;

	ConnectionState(int fd);
};

typedef boost::intrusive_ptr<ConnectionState> ConnectionStatePtr;

class Reactor
{
	int epfd;
	void* main_coro;
public:
	Reactor();
};

} // namespace febird


#endif // __febird_rpc_reactor_h__
