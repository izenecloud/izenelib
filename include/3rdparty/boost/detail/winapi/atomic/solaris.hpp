//
//  boost/detail/winapi/atomic/solaris.hpp
//
//  Copyright (c) 2004 - 2008 xushiwei (xushiweizh@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/detail/todo.htm for documentation.
//
#ifndef BOOST_DETAIL_WINAPI_ATOMIC_SOLARIS_HPP
#define BOOST_DETAIL_WINAPI_ATOMIC_SOLARIS_HPP

// -------------------------------------------------------------------------

__forceinline LONG WINAPI InterlockedIncrement(volatile LPLONG lpAddend)
{
    return atomic_inc_32_nv(lpAddend);
}

__forceinline LONG WINAPI InterlockedDecrement(volatile LPLONG lpAddend)
{
    return atomic_dec_32_nv(lpAddend);
}

// -------------------------------------------------------------------------

__forceinline LONG WINAPI InterlockedExchange(volatile LPLONG Target, LONG Value)
{
	return atomic_swap_32(Target, Value);
}

__forceinline PVOID WINAPI InterlockedExchangePointer(volatile PVOID* Target, PVOID Value)
{
	return atomic_swap_ptr(Target, Value);
}

// -------------------------------------------------------------------------

__forceinline LONG WINAPI InterlockedExchangeAdd(volatile LPLONG Addend, LONG Value)
{
    return atomic_add_32_nv(Addend, Value) - Value;
}

// -------------------------------------------------------------------------

__forceinline LONG WINAPI InterlockedCompareExchange(
    volatile LPLONG Destination, LONG Exchange, LONG Comperand)
{
	return atomic_cas_32(Destination, Comperand, Exchange);
}

__forceinline PVOID WINAPI InterlockedCompareExchangePointer(
    volatile PVOID* Destination, PVOID Exchange, PVOID Comperand)
{
	return atomic_cas_ptr(Destination, Comperand, Exchange);
}

// -------------------------------------------------------------------------
// $Log: $

#endif /* BOOST_DETAIL_WINAPI_ATOMIC_SOLARIS_HPP */
