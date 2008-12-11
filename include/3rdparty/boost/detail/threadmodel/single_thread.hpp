//
//  boost/detail/threadmodel/single_thread.hpp
//
//  Copyright (c) 2004 - 2008 xushiwei (xushiweizh@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/detail/todo.htm for documentation.
//
#ifndef BOOST_DETAIL_THREADMODEL_SINGLE_THREAD_HPP
#define BOOST_DETAIL_THREADMODEL_SINGLE_THREAD_HPP

NS_BOOST_DETAIL_BEGIN

// -------------------------------------------------------------------------
// struct refcount_st

class refcount_st
{
public:
	typedef long value_type;

private:
	value_type m_nRef;

public:
	refcount_st(value_type nRef) : m_nRef(nRef)
	{
	}

	value_type BOOST_DETAIL_CALL acquire() { return ++m_nRef; }
	value_type BOOST_DETAIL_CALL release() { return --m_nRef; }

	operator value_type()
	{
		return m_nRef;
	}
};

// -------------------------------------------------------------------------
// class critical_section_st

class critical_section_st
{
public:
	void BOOST_DETAIL_CALL acquire() {}
	void BOOST_DETAIL_CALL release() {}

#if defined(_DEBUG)
	bool BOOST_DETAIL_CALL good() const {
		return true;
	}
#endif

public:
	class scoped_lock
	{
	public:
		scoped_lock(critical_section_st& cs)
		{
		}
	};
};

// -------------------------------------------------------------------------
//	$Log: $

NS_BOOST_DETAIL_END

#endif /* BOOST_DETAIL_THREADMODEL_SINGLE_THREAD_HPP */
