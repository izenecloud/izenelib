//
//  boost/memory/thread/tls.hpp (*)
//
//  Copyright (c) 2004 - 2008 xushiwei (xushiweizh@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/memory/index.htm for documentation.
//
#ifndef BOOST_MEMORY_THREAD_TLS_HPP
#define BOOST_MEMORY_THREAD_TLS_HPP

#ifndef BOOST_MEMORY_BASIC_HPP
#include "../basic.hpp"
#endif

#ifndef BOOST_DETAIL_THREADMODEL_HPP
#include <boost/detail/threadmodel.hpp>
#endif

// -------------------------------------------------------------------------
// class win_tls_key

#if defined(_WIN32)

NS_BOOST_MEMORY_BEGIN

typedef DWORD TLSINDEX;

class win_tls_key
{
private:
	TLSINDEX m_key;

public:
	win_tls_key()
		: m_key(TLS_OUT_OF_INDEXES) {
	}

	void BOOST_MEMORY_CALL create() {
		m_key = TlsAlloc();
	}

	void BOOST_MEMORY_CALL clear() {
		TlsFree(m_key);
	}

	void BOOST_MEMORY_CALL put(void* p) const {
		TlsSetValue(m_key, p);
	}

	void* BOOST_MEMORY_CALL get() const {
		return TlsGetValue(m_key);
	}
};

NS_BOOST_MEMORY_END

#endif // defined(_WIN32)

// -------------------------------------------------------------------------
// class pthread_tls_key

#if !defined(_WIN32)

#ifndef _PTHREAD_H
#include <pthread.h>
#endif

NS_BOOST_MEMORY_BEGIN

class pthread_tls_key
{
private:
	pthread_key_t m_key;

public:
	pthread_tls_key()
		: m_key(TLS_OUT_OF_INDEXES) {
	}

	void BOOST_MEMORY_CALL create() {
		pthread_key_create(&m_key, NULL);
	}

	void BOOST_MEMORY_CALL clear() const {
		pthread_key_delete(m_key);
	}

	void BOOST_MEMORY_CALL put(void* p) const {
		pthread_setspecific(m_key, p);
	}

	void* BOOST_MEMORY_CALL get() const {
		return pthread_getspecific(m_key);
	}
};

NS_BOOST_MEMORY_END

#endif // !defined(_WIN32)

// -------------------------------------------------------------------------
// class tls_key

NS_BOOST_MEMORY_BEGIN

#if defined(_WIN32)

typedef win_tls_key tls_key;

#else

typedef pthread_tls_key tls_key;

#endif

NS_BOOST_MEMORY_END

// -------------------------------------------------------------------------
// class tls_ptr - removed, use boost::thread_specific_ptr instead of tls_ptr

// -------------------------------------------------------------------------
// class tls_object

NS_BOOST_MEMORY_BEGIN

template <class Type>
class tls_factory
{
public:
	enum { has_cleanup = 1 };
	
	static Type* BOOST_MEMORY_CALL create() {
		return new Type;
	}
	static void cleanup(void* p) {
		delete (Type*)p;
	}
};

#if defined(_MSC_VER)
#pragma warning(disable:4786)
#endif

template <
	class Type, 
	class Factory = tls_factory<Type>,
	class ThreadModel = NS_BOOST_DETAIL::default_threadmodel>
class tls_object
{
private:
	typedef typename ThreadModel::refcount refcount;

	tls_key m_key;
	refcount m_ref;

public:
	tls_object() : m_ref(0) {}

	void BOOST_MEMORY_CALL init()
	{
		if (m_ref.acquire() == 1) {
			m_key.create();
		}
	}

	void BOOST_MEMORY_CALL term()
	{
		if (m_ref.release() == 0)
		{
			if (Factory::has_cleanup)
			{
				void* p = m_key.get();
				if (p)
					Factory::cleanup(p);
			}
			m_key.clear();
		}
	}

	const tls_key& BOOST_MEMORY_CALL storage() const
	{
		return m_key;
	}

	Type& BOOST_MEMORY_CALL get()
	{
		void* p = m_key.get();
		if (p == NULL) {
			m_key.put(p = Factory::create());
			BOOST_MEMORY_ASSERT(m_key.get() == p);
		}
		return *(Type*)p;
	}
};

NS_BOOST_MEMORY_END

// -------------------------------------------------------------------------
//	$Log: $

#endif /* BOOST_MEMORY_THREAD_TLS_HPP */
