/*
 * Copyright (c) 2006, Aleksey Cheusov <vle@gmx.net>
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation.  I make no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 */

#ifndef _JUDY_COMMON_H_
#define _JUDY_COMMON_H_

#include "Judy.h"

////////////////////////////////////////////////////////////
///  defining JUDYARRAY_DEBUGINFO
#ifdef JUDYARRAY_NO_DEBUGINFO

#ifdef JUDYARRAY_DEBUGINFO
#undef JUDYARRAY_DEBUGINFO
#endif // JUDYARRAY_DEBUGINFO

#else

#if !defined(JUDYARRAY_DEBUGINFO) && !defined(NDEBUG)
#define JUDYARRAY_DEBUGINFO
#endif // JUDYARRAY_DEBUGINFO && NDEBUG

#endif // JUDYARRAY_NO_DEBUGINFO
////////////////////////////////////////////////////////////

#define __JUDYARRAY_TYPEDEFS(macrosarg_from)                             \
	typedef typename macrosarg_from::key_type        key_type;           \
	typedef typename macrosarg_from::data_type       data_type;          \
	typedef typename macrosarg_from::mapped_type     mapped_type;        \
	typedef typename macrosarg_from::value_type      value_type;         \
	typedef typename macrosarg_from::size_type       size_type;          \
	typedef typename macrosarg_from::difference_type difference_type;    \
    \
	typedef typename macrosarg_from::pointer         pointer;            \
	typedef typename macrosarg_from::const_pointer   const_pointer;      \
	typedef typename macrosarg_from::reference       reference;          \
	typedef typename macrosarg_from::const_reference const_reference;


struct __judy_always_zero {
	Word_t operator () (int) const
	{
		return 0;
	}
};

template <typename T>
class judy_reference {
private:
	T *m_pointer;

public:
	judy_reference ()
	{
		m_pointer = NULL;
	}

	judy_reference (const judy_reference &a)
	{
		m_pointer = a.m_pointer;
	}

	~judy_reference ()
	{
	}

	judy_reference &operator = (const judy_reference &a)
	{
		// copy value, but the pointer!!!
		assert (a.m_pointer);
		assert (m_pointer);

		*m_pointer = *a.m_pointer;

		return *this;
	}

	operator T () const
	{
		assert (m_pointer);
		return *m_pointer;
	}

	void set_pointer (T *pointer)
	{
		m_pointer = pointer;
	}

	template <typename TValue>
	judy_reference &operator = (const TValue& v)
	{
		assert (m_pointer);
		*m_pointer = v;
	}
};

#endif // _JUDY_COMMON_H_
