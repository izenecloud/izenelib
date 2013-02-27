//
//  boost/detail/winapi/atomic/atomic_builtins.hpp
//
//  Copyright (c) 2004 - 2008 xushiwei (xushiweizh@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/detail/todo.htm for documentation.
//
#ifndef BOOST_DETAIL_WINAPI_ATOMIC_ATOMIC_BUILTINS_HPP
#define BOOST_DETAIL_WINAPI_ATOMIC_ATOMIC_BUILTINS_HPP

// -------------------------------------------------------------------------

inline long InterlockedIncrement(volatile long * lpAddend)
{
    return __sync_add_and_fetch(lpAddend, 1);
}

inline long InterlockedDecrement(volatile long * lpAddend)
{
    return __sync_sub_and_fetch(lpAddend, 1);
}
// -------------------------------------------------------------------------
// $Log: $

#endif /* BOOST_DETAIL_WINAPI_ATOMIC_ATOMIC_BUILTINS_HPP */

