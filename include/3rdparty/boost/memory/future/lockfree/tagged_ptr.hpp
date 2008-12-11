//
//  boost/lockfree/tagged_ptr.hpp
//
//  Copyright (c) 2004 - 2008 xushiwei (xushiweizh@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/lockfree/todo.htm for documentation.
//
#ifndef BOOST_LOCKFREE_TAGGED_PTR_HPP
#define BOOST_LOCKFREE_TAGGED_PTR_HPP

#ifndef BOOST_DETAIL_WINAPI_WINBASE_H
#include <boost/detail/winapi/winbase.h>
#endif

#ifndef BOOST_DETAIL_DEBUG_HPP
#include <boost/detail/debug.hpp>
#endif

// -------------------------------------------------------------------------

#ifndef NS_BOOST_LOCKFREE_BEGIN
#define NS_BOOST_LOCKFREE_BEGIN		namespace boost { namespace lockfree {
#define NS_BOOST_LOCKFREE_END		} }
#define NS_BOOST_LOCKFREE			boost::lockfree
#endif

#ifndef BOOST_LOCKFREE_CALL
#define BOOST_LOCKFREE_CALL
#endif

NS_BOOST_LOCKFREE_BEGIN

// -------------------------------------------------------------------------
// class tagged_ptr

template <class E>
class tagged_ptr
{
private:
	typedef E* Type;
	Type m_p[2];

public:
	tagged_ptr(Type vInit = Type()) {
		m_p[0] = vInit;
	}
	
	template <class FuncT>
	bool BOOST_LOCKFREE_CALL set(FuncT& op)
	{
		for (;;)
		{
			Type vTag = m_p[1]; // NOTE: getting 'tag' before getting 'data'!
			Type vOld = m_p[0];
			if (!op.valid(vOld))
				return false;

			Type vNew = op(vOld);
			if (TaggedCompareAndSwapPointer(m_p, vOld, vNew, vTag))
				return true;
		}
	}

	__forceinline Type BOOST_LOCKFREE_CALL clear()
	{
		return (Type)InterlockedExchangePointer((PVOID*)m_p, NULL);
	}

	__forceinline Type BOOST_LOCKFREE_CALL get() const
	{
		return m_p[0];
	}
};

// -------------------------------------------------------------------------
// $Log: $

NS_BOOST_LOCKFREE_END

#endif /* BOOST_LOCKFREE_TAGGED_PTR_HPP */
