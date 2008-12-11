//
//  boost/memory/system_alloc.hpp
//
//  Copyright (c) 2004 - 2008 xushiwei (xushiweizh@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/memory/index.htm for documentation.
//
#ifndef BOOST_MEMORY_SYSTEM_ALLOC_HPP
#define BOOST_MEMORY_SYSTEM_ALLOC_HPP

#include "basic.hpp"

#include <boost/detail/threadmodel.hpp>
#include <algorithm> // std::swap

#include <cstdlib> // malloc, free

#include "system_pool.hpp"

NS_BOOST_MEMORY_BEGIN

// -------------------------------------------------------------------------
// BOOST_MEMORY_FAKE_DBG_ALLOCATE_

#if defined(_DEBUG)

#define BOOST_MEMORY_FAKE_DBG_ALLOCATE_()												\
	void* BOOST_MEMORY_CALL allocate(size_t cb, LPCSTR szFile, int nLine)				\
		{ return allocate(cb); }														\
	void* BOOST_MEMORY_CALL allocate(size_t cb, destructor_t fn, LPCSTR szFile, int nLine)	\
		{ return allocate(cb, fn); }													\
	void* BOOST_MEMORY_CALL allocate(size_t cb, int fnZero, LPCSTR szFile, int nLine)	\
		{ return allocate(cb); }														\
	void* BOOST_MEMORY_CALL reallocate(void* p, size_t oldSize, size_t newSize,			\
		LPCSTR szFile, int nLine) { return reallocate(p, oldSize, newSize); }			\
	template <class Type>																\
	Type* BOOST_MEMORY_CALL newArray(size_t count, Type* zero, LPCSTR szFile, int nLine)\
		{ return newArray(count, zero); }

#else

#define BOOST_MEMORY_FAKE_DBG_ALLOCATE_()

#endif

// -------------------------------------------------------------------------
// class stdlib_alloc

#if defined(__GNUG__)
#define _msize	malloc_usable_size
#endif

#ifdef __APPLE__
#undef _msize
#define _msize  malloc_size
#endif

class stdlib_alloc
{
public:
	static void* BOOST_MEMORY_CALL allocate(size_t cb)					{ return malloc(cb); }
	static void* BOOST_MEMORY_CALL allocate(size_t cb, destructor_t fn)	{ return malloc(cb); }
	static void* BOOST_MEMORY_CALL allocate(size_t cb, int fnZero)		{ return malloc(cb); }

	static void* BOOST_MEMORY_CALL reallocate(void* p, size_t oldSize, size_t newSize) {
		return realloc(p, newSize);
	}

	static void BOOST_MEMORY_CALL deallocate(void* p)			{ free(p); }
	static void BOOST_MEMORY_CALL deallocate(void* p, size_t)	{ free(p); }
	static void BOOST_MEMORY_CALL swap(stdlib_alloc& o)			{}

	static size_t BOOST_MEMORY_CALL alloc_size(void* p)
	{
		return _msize(p);
	}

	template <class Type>
	static void BOOST_MEMORY_CALL destroy(Type* obj)
	{
		obj->~Type();
		free(obj);
	}

	template <class Type>
	static Type* BOOST_MEMORY_CALL newArray(size_t count, Type* zero)
	{
		Type* array = (Type*)malloc(sizeof(Type) * count);
		return constructor_traits<Type>::constructArray(array, count);
	}

	template <class Type>
	static void BOOST_MEMORY_CALL destroyArray(Type* array, size_t count)
	{
		destructor_traits<Type>::destructArrayN(array, count);
		free(array);
	}

#if defined(_malloc_dbg)
	static void* BOOST_MEMORY_CALL allocate(size_t cb, LPCSTR szFile, int nLine)
		{ return _malloc_dbg(cb, _NORMAL_BLOCK, szFile, nLine); }

	static void* BOOST_MEMORY_CALL allocate(size_t cb, destructor_t fn, LPCSTR szFile, int nLine)
		{ return _malloc_dbg(cb, _NORMAL_BLOCK, szFile, nLine); }
	
	static void* BOOST_MEMORY_CALL allocate(size_t cb, int fnZero, LPCSTR szFile, int nLine)
		{ return _malloc_dbg(cb, _NORMAL_BLOCK, szFile, nLine); }

	static void* BOOST_MEMORY_CALL reallocate(void* p, size_t oldSize, size_t newSize, LPCSTR szFile, int nLine)
		{ return _realloc_dbg(p, newSize, _NORMAL_BLOCK, szFile, nLine); }

	template <class Type>
	static Type* BOOST_MEMORY_CALL newArray(size_t count, Type* zero, LPCSTR szFile, int nLine)
	{
		Type* array = (Type*)_malloc_dbg(sizeof(Type) * count, _NORMAL_BLOCK, szFile, nLine);
		return constructor_traits<Type>::constructArray(array, count);
	}
#else
	BOOST_MEMORY_FAKE_DBG_ALLOCATE_()
#endif
};

typedef system_pool_s<stdlib_alloc, default_stack> system_pool;

class system_pool_dyn
{
public:
	static void* BOOST_MEMORY_CALL allocate(size_t cb) { return  system_pool::allocate(cb); }
	static void BOOST_MEMORY_CALL deallocate(void* p) { system_pool::deallocate(p); }
	static size_t BOOST_MEMORY_CALL alloc_size(void* p) {
		return system_pool::alloc_size(p);
	}
};
#if defined(BOOST_MEMORY_NO_SYSTEM_POOL)
#define BOOST_MEMORY_NO_SYSTEM_POOL_DYN_
#endif

#if !defined(BOOST_MEMORY_NO_SYSTEM_POOL)
typedef system_pool_dyn system_alloc;
#else
typedef stdlib_alloc system_alloc;
#endif


// -------------------------------------------------------------------------
// $Log: $

NS_BOOST_MEMORY_END

#endif /* BOOST_MEMORY_SYSTEM_ALLOC_HPP */
