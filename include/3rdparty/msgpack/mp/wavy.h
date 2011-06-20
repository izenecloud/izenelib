//
// mpio wavy
//
// Copyright (C) 2008-2010 FURUHASHI Sadayuki
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//
#ifndef MP_WAVY_H__
#define MP_WAVY_H__

#include "functional.h"
#include "memory.h"
#include "pthread.h"
#include "object_delete.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <memory>

namespace mp {
namespace wavy {


class basic_handler;
class handler;
class event;
class xfer;

typedef shared_ptr<basic_handler> shared_handler;
typedef weak_ptr<basic_handler> weak_handler;


class loop {
public:
	typedef mp::wavy::basic_handler basic_handler;
	typedef mp::wavy::handler handler;
	typedef mp::wavy::event event;

	loop();
	loop(function<void ()> thread_init_func);

	~loop();

	void start(size_t num);

	void run(size_t num);   // run = start + join

	bool is_running() const;

	void run_once();
	void run_nonblock();

	void end();
	bool is_end() const;

	void join();
	void detach();

	void add_thread(size_t num);


	void remove_handler(int fd);


	typedef function<void (int fd, int err)> connect_callback_t;

	void connect(
			int socket_family, int socket_type, int protocol,
			const sockaddr* addr, socklen_t addrlen,
			const timespec* timeout, connect_callback_t callback);

	void connect(
			int socket_family, int socket_type, int protocol,
			const sockaddr* addr, socklen_t addrlen,
			double timeout_sec, connect_callback_t callback);


	typedef function<void (int fd, int err)> listen_callback_t;

	int listen(
			int socket_family, int socket_type, int protocol,
			const sockaddr* addr, socklen_t addrlen,
			listen_callback_t callback,
			int backlog = 1024);


	int add_timer(const timespec* value, const timespec* interval,
			function<bool ()> callback);

	int add_timer(double value_sec, double interval_sec,
			function<bool ()> callback);

	void remove_timer(int ident);


	int add_signal(int signo, function<bool ()> callback);

	void remove_signal(int ident);



	typedef void (*finalize_t)(void* user);


	void write(int fd, const void* buf, size_t size);

	void write(int fd, const void* buf, size_t size,
			finalize_t fin, void* user);

	template <typename T>
	void write(int fd, const void* buf, size_t size,
			std::auto_ptr<T>& fin);

	template <typename T>
	void write(int fd, const void* buf, size_t size,
			mp::shared_ptr<T> fin);


	void writev(int fd, const struct iovec* vec, size_t veclen,
			finalize_t fin, void* user);

	template <typename T>
	void writev(int fd, const struct iovec* vec, size_t veclen,
			std::auto_ptr<T>& fin);

	template <typename T>
	void writev(int fd, const struct iovec* vec, size_t veclen,
			mp::shared_ptr<T> fin);


	void sendfile(int fd, int infd, uint64_t off, size_t size,
			finalize_t fin, void* user);
	
	void hsendfile(int fd,
			const void* header, size_t header_size,
			int infd, uint64_t off, size_t size,
			finalize_t fin, void* user);
	
	void hvsendfile(int fd,
			const struct iovec* header_vec, size_t header_veclen,
			int infd, uint64_t off, size_t size,
			finalize_t fin, void* user);


	void commit(int fd, xfer* xf);


	void flush();


	template <typename Handler>
	shared_ptr<Handler> add_handler();
	template <typename Handler, typename A1>
	shared_ptr<Handler> add_handler(A1 a1);
	template <typename Handler, typename A1, typename A2>
	shared_ptr<Handler> add_handler(A1 a1, A2 a2);
	template <typename Handler, typename A1, typename A2, typename A3>
	shared_ptr<Handler> add_handler(A1 a1, A2 a2, A3 a3);
	template <typename Handler, typename A1, typename A2, typename A3, typename A4>
	shared_ptr<Handler> add_handler(A1 a1, A2 a2, A3 a3, A4 a4);
	template <typename Handler, typename A1, typename A2, typename A3, typename A4, typename A5>
	shared_ptr<Handler> add_handler(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5);
	template <typename Handler, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	shared_ptr<Handler> add_handler(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6);
	template <typename Handler, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
	shared_ptr<Handler> add_handler(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7);
	template <typename Handler, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
	shared_ptr<Handler> add_handler(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8);
	template <typename Handler, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
	shared_ptr<Handler> add_handler(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9);
	template <typename Handler, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
	shared_ptr<Handler> add_handler(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10);
	template <typename Handler, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11>
	shared_ptr<Handler> add_handler(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11);
	template <typename Handler, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12>
	shared_ptr<Handler> add_handler(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12);
	template <typename Handler, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13>
	shared_ptr<Handler> add_handler(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13);
	template <typename Handler, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14>
	shared_ptr<Handler> add_handler(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14);
	template <typename Handler, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15>
	shared_ptr<Handler> add_handler(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15);
	template <typename Handler, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15, typename A16>
	shared_ptr<Handler> add_handler(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16);


	template <typename F>
	void submit(F f);
	template <typename F, typename A1>
	void submit(F f, A1 a1);
	template <typename F, typename A1, typename A2>
	void submit(F f, A1 a1, A2 a2);
	template <typename F, typename A1, typename A2, typename A3>
	void submit(F f, A1 a1, A2 a2, A3 a3);
	template <typename F, typename A1, typename A2, typename A3, typename A4>
	void submit(F f, A1 a1, A2 a2, A3 a3, A4 a4);
	template <typename F, typename A1, typename A2, typename A3, typename A4, typename A5>
	void submit(F f, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5);
	template <typename F, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	void submit(F f, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6);
	template <typename F, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
	void submit(F f, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7);
	template <typename F, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
	void submit(F f, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8);
	template <typename F, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
	void submit(F f, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9);
	template <typename F, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
	void submit(F f, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10);
	template <typename F, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11>
	void submit(F f, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11);
	template <typename F, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12>
	void submit(F f, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12);
	template <typename F, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13>
	void submit(F f, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13);
	template <typename F, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14>
	void submit(F f, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14);
	template <typename F, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15>
	void submit(F f, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15);
	template <typename F, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15, typename A16>
	void submit(F f, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16);


private:
	shared_handler add_handler_impl(shared_handler sh);

	typedef function<void ()> task_t;
	void submit_impl(task_t f);

private:
	void* m_impl;

	loop(const loop&);
};


class event {
protected:
	event() { }
	~event() { }
public:
	void more();
	void next();
	void remove();
private:
	event(const event&);
};


class xfer {
public:
	xfer();
	~xfer();

	typedef loop::finalize_t finalize_t;

	void push_write(const void* buf, size_t size);

	void push_writev(const struct iovec* vec, size_t veclen);

	void push_sendfile(int infd, uint64_t off, size_t len);

	void push_finalize(finalize_t fin, void* user);

	template <typename T>
	void push_finalize(std::auto_ptr<T> fin);

	template <typename T>
	void push_finalize(mp::shared_ptr<T> fin);

	bool empty() const;

	void clear();

	void migrate(xfer* to);

protected:
	char* m_head;
	char* m_tail;
	size_t m_free;

	void reserve(size_t reqsz);

private:
	xfer(const xfer&);
};


struct basic_handler {
public:
	typedef bool (*callback_t)(basic_handler*, event&);

	template <typename IMPL>
	basic_handler(int ident, IMPL* self) :
		m_ident(ident), m_callback(&static_callback<IMPL>) { }

	basic_handler(int ident, callback_t callback) :
		m_ident(ident), m_callback(callback) { }

	virtual ~basic_handler() { }

	int ident() const { return m_ident; }

	int fd() const { return ident(); }

	bool operator() (event& e);

private:
	int m_ident;

	callback_t m_callback;

private:
	template <typename IMPL>
	static bool static_callback(basic_handler* self, event& e)
	{
		return (*static_cast<IMPL*>(self))(e);
	}

	basic_handler();
	basic_handler(const basic_handler&);
};


struct handler : public mp::enable_shared_from_this<handler>, public basic_handler {
public:
	handler(int fd) : basic_handler(fd, &callback_on_read) { }

	~handler() { ::close(fd()); }

	virtual void on_read(event& e) = 0;

public:
	template <typename IMPL>
	shared_ptr<IMPL> shared_self()
	{
		return static_pointer_cast<IMPL>(enable_shared_from_this<handler>::shared_from_this());
	}

	template <typename IMPL>
	shared_ptr<IMPL const> shared_self() const
	{
		return static_pointer_cast<IMPL>(enable_shared_from_this<handler>::shared_from_this());
	}

private:
	static inline bool callback_on_read(basic_handler* self, event& e)
	{
		static_cast<handler*>(self)->on_read(e);
		return true;
	}
	friend class basic_handler;
};


inline bool basic_handler::operator() (event& e)
{
	if(m_callback == handler::callback_on_read) {
		return handler::callback_on_read(this, e);
	} else {
		return m_callback(this, e);
	}
}


template <typename Handler>
shared_ptr<Handler> loop::add_handler()
	{ return static_pointer_cast<Handler>(add_handler_impl(
			shared_ptr<Handler>(new Handler())) ); }
template <typename Handler, typename A1>
shared_ptr<Handler> loop::add_handler(A1 a1)
	{ return static_pointer_cast<Handler>(add_handler_impl(
			shared_ptr<Handler>(new Handler(a1))) ); }
template <typename Handler, typename A1, typename A2>
shared_ptr<Handler> loop::add_handler(A1 a1, A2 a2)
	{ return static_pointer_cast<Handler>(add_handler_impl(
			shared_ptr<Handler>(new Handler(a1, a2))) ); }
template <typename Handler, typename A1, typename A2, typename A3>
shared_ptr<Handler> loop::add_handler(A1 a1, A2 a2, A3 a3)
	{ return static_pointer_cast<Handler>(add_handler_impl(
			shared_ptr<Handler>(new Handler(a1, a2, a3))) ); }
template <typename Handler, typename A1, typename A2, typename A3, typename A4>
shared_ptr<Handler> loop::add_handler(A1 a1, A2 a2, A3 a3, A4 a4)
	{ return static_pointer_cast<Handler>(add_handler_impl(
			shared_ptr<Handler>(new Handler(a1, a2, a3, a4))) ); }
template <typename Handler, typename A1, typename A2, typename A3, typename A4, typename A5>
shared_ptr<Handler> loop::add_handler(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
	{ return static_pointer_cast<Handler>(add_handler_impl(
			shared_ptr<Handler>(new Handler(a1, a2, a3, a4, a5))) ); }
template <typename Handler, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
shared_ptr<Handler> loop::add_handler(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
	{ return static_pointer_cast<Handler>(add_handler_impl(
			shared_ptr<Handler>(new Handler(a1, a2, a3, a4, a5, a6))) ); }
template <typename Handler, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
shared_ptr<Handler> loop::add_handler(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
	{ return static_pointer_cast<Handler>(add_handler_impl(
			shared_ptr<Handler>(new Handler(a1, a2, a3, a4, a5, a6, a7))) ); }
template <typename Handler, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
shared_ptr<Handler> loop::add_handler(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
	{ return static_pointer_cast<Handler>(add_handler_impl(
			shared_ptr<Handler>(new Handler(a1, a2, a3, a4, a5, a6, a7, a8))) ); }
template <typename Handler, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
shared_ptr<Handler> loop::add_handler(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
	{ return static_pointer_cast<Handler>(add_handler_impl(
			shared_ptr<Handler>(new Handler(a1, a2, a3, a4, a5, a6, a7, a8, a9))) ); }
template <typename Handler, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
shared_ptr<Handler> loop::add_handler(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)
	{ return static_pointer_cast<Handler>(add_handler_impl(
			shared_ptr<Handler>(new Handler(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10))) ); }
template <typename Handler, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11>
shared_ptr<Handler> loop::add_handler(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)
	{ return static_pointer_cast<Handler>(add_handler_impl(
			shared_ptr<Handler>(new Handler(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11))) ); }
template <typename Handler, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12>
shared_ptr<Handler> loop::add_handler(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)
	{ return static_pointer_cast<Handler>(add_handler_impl(
			shared_ptr<Handler>(new Handler(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12))) ); }
template <typename Handler, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13>
shared_ptr<Handler> loop::add_handler(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)
	{ return static_pointer_cast<Handler>(add_handler_impl(
			shared_ptr<Handler>(new Handler(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13))) ); }
template <typename Handler, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14>
shared_ptr<Handler> loop::add_handler(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)
	{ return static_pointer_cast<Handler>(add_handler_impl(
			shared_ptr<Handler>(new Handler(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14))) ); }
template <typename Handler, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15>
shared_ptr<Handler> loop::add_handler(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)
	{ return static_pointer_cast<Handler>(add_handler_impl(
			shared_ptr<Handler>(new Handler(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15))) ); }
template <typename Handler, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15, typename A16>
shared_ptr<Handler> loop::add_handler(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)
	{ return static_pointer_cast<Handler>(add_handler_impl(
			shared_ptr<Handler>(new Handler(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16))) ); }

template <typename F>
inline void loop::submit(F f)
	{ submit_impl(task_t(f)); }
template <typename F, typename A1>
inline void loop::submit(F f, A1 a1)
	{ submit_impl(bind(f, a1)); }
template <typename F, typename A1, typename A2>
inline void loop::submit(F f, A1 a1, A2 a2)
	{ submit_impl(bind(f, a1, a2)); }
template <typename F, typename A1, typename A2, typename A3>
inline void loop::submit(F f, A1 a1, A2 a2, A3 a3)
	{ submit_impl(bind(f, a1, a2, a3)); }
template <typename F, typename A1, typename A2, typename A3, typename A4>
inline void loop::submit(F f, A1 a1, A2 a2, A3 a3, A4 a4)
	{ submit_impl(bind(f, a1, a2, a3, a4)); }
template <typename F, typename A1, typename A2, typename A3, typename A4, typename A5>
inline void loop::submit(F f, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
	{ submit_impl(bind(f, a1, a2, a3, a4, a5)); }
template <typename F, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
inline void loop::submit(F f, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
	{ submit_impl(bind(f, a1, a2, a3, a4, a5, a6)); }
template <typename F, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
inline void loop::submit(F f, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
	{ submit_impl(bind(f, a1, a2, a3, a4, a5, a6, a7)); }
template <typename F, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
inline void loop::submit(F f, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
	{ submit_impl(bind(f, a1, a2, a3, a4, a5, a6, a7, a8)); }
template <typename F, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
inline void loop::submit(F f, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
	{ submit_impl(bind(f, a1, a2, a3, a4, a5, a6, a7, a8, a9)); }
template <typename F, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
inline void loop::submit(F f, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)
	{ submit_impl(bind(f, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)); }
template <typename F, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11>
inline void loop::submit(F f, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11)
	{ submit_impl(bind(f, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11)); }
template <typename F, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12>
inline void loop::submit(F f, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12)
	{ submit_impl(bind(f, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12)); }
template <typename F, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13>
inline void loop::submit(F f, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13)
	{ submit_impl(bind(f, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13)); }
template <typename F, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14>
inline void loop::submit(F f, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14)
	{ submit_impl(bind(f, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14)); }
template <typename F, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15>
inline void loop::submit(F f, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15)
	{ submit_impl(bind(f, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15)); }
template <typename F, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15, typename A16>
inline void loop::submit(F f, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16)
	{ submit_impl(bind(f, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16)); }


inline xfer::xfer() :
	m_head(NULL), m_tail(NULL), m_free(0) { }

inline xfer::~xfer()
{
	if(m_head) {
		clear();
		::free(m_head);
	}
}

inline bool xfer::empty() const
{
	return m_head == m_tail;
}

template <typename T>
inline void xfer::push_finalize(std::auto_ptr<T> fin)
{
	push_finalize(&mp::object_delete<T>, reinterpret_cast<void*>(fin.get()));
	fin.release();
}

template <typename T>
inline void xfer::push_finalize(mp::shared_ptr<T> fin)
{
	std::auto_ptr<mp::shared_ptr<T> > afin(new mp::shared_ptr<T>(fin));
	push_finalize(afin);
}

template <typename T>
inline void loop::write(int fd, const void* buf, size_t size,
		std::auto_ptr<T>& fin)
{
	write(fd, buf, size, &mp::object_delete<T>, fin.get());
	fin.release();
}

template <typename T>
inline void loop::write(int fd, const void* buf, size_t size,
		mp::shared_ptr<T> fin)
{
	std::auto_ptr<mp::shared_ptr<T> > afin(new mp::shared_ptr<T>(fin));
	write(fd, buf, size, afin);
}

template <typename T>
inline void loop::writev(int fd, const struct iovec* vec, size_t veclen,
		std::auto_ptr<T>& fin)
{
	writev(fd, vec, veclen, &mp::object_delete<T>, fin.get());
	fin.release();
}

template <typename T>
inline void loop::writev(int fd, const struct iovec* vec, size_t veclen,
		mp::shared_ptr<T> fin)
{
	std::auto_ptr<mp::shared_ptr<T> > afin(new mp::shared_ptr<T>(fin));
	writev(fd, vec, veclen, afin);
}


}  // namespace wavy
}  // namespace mp

#endif /* mp/wavy.h */

