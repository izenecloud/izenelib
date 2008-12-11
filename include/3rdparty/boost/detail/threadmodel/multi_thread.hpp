//
//  boost/detail/threadmodel/multi_thread.hpp
//
//  Copyright (c) 2004 - 2008 xushiwei (xushiweizh@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/detail/todo.htm for documentation.
//
#ifndef BOOST_DETAIL_THREADMODEL_MULTI_THREAD_HPP
#define BOOST_DETAIL_THREADMODEL_MULTI_THREAD_HPP

NS_BOOST_DETAIL_BEGIN

// -------------------------------------------------------------------------
// class scoped_lock

template <class LockT>
class scoped_lock
{
private:
	LockT& m_lock;

private:
	scoped_lock(const scoped_lock&);
	void operator=(const scoped_lock&);

public:
	scoped_lock(LockT& lock) : m_lock(lock)
	{
		m_lock.acquire();
	}
	~scoped_lock()
	{
		m_lock.release();
	}
};

// -------------------------------------------------------------------------
// class refcount_mt

class refcount_mt
{
public:
	typedef LONG value_type;

private:
	value_type m_nRef;

public:
	refcount_mt(value_type nRef)
		: m_nRef(nRef)
	{
	}

	value_type BOOST_DETAIL_CALL acquire()
	{
		return InterlockedIncrement(&m_nRef);
	}

	value_type BOOST_DETAIL_CALL release()
	{
		return InterlockedDecrement(&m_nRef);
	}

	operator value_type()
	{
		return m_nRef;
	}
};

// -------------------------------------------------------------------------
// class critical_section_mt

class critical_section_mt
{
private:
	CRITICAL_SECTION m_cs;
	
private:
	critical_section_mt(const critical_section_mt&);
	void operator=(const critical_section_mt&);

public:
	critical_section_mt()
	{
		InitializeCriticalSection(&m_cs);
	}
	~critical_section_mt()
	{
		DeleteCriticalSection(&m_cs);
	}

	void BOOST_DETAIL_CALL acquire()
	{
		EnterCriticalSection(&m_cs);
	}

	void BOOST_DETAIL_CALL release()
	{
		LeaveCriticalSection(&m_cs);
	}

#if defined(_DEBUG)
	bool BOOST_DETAIL_CALL good() const // debug only
	{
		const char* p = (const char*)&m_cs;
		for (size_t i = 0; i < sizeof(m_cs); ++i)
		{
			if (p[i] != 0)
				return true;
		}
		return false;
	}
#endif

public:
	typedef NS_BOOST_DETAIL::scoped_lock<critical_section_mt> scoped_lock;
};

// -------------------------------------------------------------------------
//	$Log: $

NS_BOOST_DETAIL_END

#endif /* BOOST_DETAIL_THREADMODEL_MULTI_THREAD_HPP */
