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

#include <algorithm> // std::swap

#include <cstdlib> // malloc, free
#include <cstring> // memcpy

NS_BOOST_MEMORY_BEGIN

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
    static void* allocate(size_t cb)
    {
        return malloc(cb);
    }
    static void* allocate(size_t cb, destructor_t fn)
    {
        return malloc(cb);
    }
    static void* allocate(size_t cb, int fnZero)
    {
        return malloc(cb);
    }

    static void* reallocate(void* p, size_t oldSize, size_t newSize)
    {
        return realloc(p, newSize);
    }

    static void deallocate(void* p)
    {
        free(p);
    }
    static void deallocate(void* p, size_t)
    {
        free(p);
    }
    static void swap(stdlib_alloc& o)			{}

    static size_t alloc_size(void* p)
    {
        return _msize(p);
    }

    template <class Type>
    static void destroy(Type* obj)
    {
        obj->~Type();
        free(obj);
    }

    template <class Type>
    static Type* newArray(size_t count, Type* zero)
    {
        Type* array = (Type*)malloc(sizeof(Type) * count);
        return constructor_traits<Type>::constructArray(array, count);
    }

    template <class Type>
    static void destroyArray(Type* array, size_t count)
    {
        destructor_traits<Type>::destructArrayN(array, count);
        free(array);
    }
};

typedef stdlib_alloc system_alloc;


// -------------------------------------------------------------------------
// $Log: $

NS_BOOST_MEMORY_END

#endif /* BOOST_MEMORY_SYSTEM_ALLOC_HPP */
