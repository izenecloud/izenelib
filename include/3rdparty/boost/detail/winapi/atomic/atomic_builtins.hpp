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

__forceinline LONG WINAPI InterlockedIncrement(volatile LPLONG lpAddend)
{
	return __sync_add_and_fetch(lpAddend, 1);
}

__forceinline LONG WINAPI InterlockedDecrement(volatile LPLONG lpAddend)
{
    return __sync_sub_and_fetch(lpAddend, 1);
}

// -------------------------------------------------------------------------

__forceinline LONG WINAPI InterlockedExchange(volatile LPLONG Target, LONG Value)
{
	__sync_synchronize();
	return __sync_lock_test_and_set(Target, Value);
}

__forceinline PVOID WINAPI InterlockedExchangePointer(volatile PVOID* Target, PVOID Value)
{
	__sync_synchronize();
	return (PVOID)__sync_lock_test_and_set(Target, Value);
}

// -------------------------------------------------------------------------

__forceinline LONG WINAPI InterlockedExchangeAdd(volatile LPLONG Addend, LONG Value)
{
    return __sync_fetch_and_add(Addend, Value);
}

// -------------------------------------------------------------------------

__forceinline LONG WINAPI InterlockedCompareExchange(
    volatile LPLONG Destination, LONG Exchange, LONG Comperand)
{
	return __sync_val_compare_and_swap(Destination, Comperand, Exchange);
}

__forceinline PVOID WINAPI InterlockedCompareExchangePointer(
    volatile PVOID* Destination, PVOID Exchange, PVOID Comperand)
{
	return (PVOID)__sync_val_compare_and_swap(Destination, Comperand, Exchange);
}

// -------------------------------------------------------------------------
// InterlockedExchange64

__forceinline LONG64 WINAPI InterlockedExchange64(
	volatile PLONG64 Target, LONG64 Value)
{
	__sync_synchronize();
	return __sync_lock_test_and_set(Target, Value);
}

// -------------------------------------------------------------------------
// CompareAndSwap64

__forceinline bool WINAPI CompareAndSwap64(
	volatile PLONG64 Destination, LONG64 Comperand, LONG64 Exchange)
{
	return __sync_bool_compare_and_swap_8(Destination, Comperand, Exchange);
}

// -------------------------------------------------------------------------
// CompareAndSwap128

#if defined(__GNUC__) && defined(__x86_64__) &&                       \
    ( __GCC_HAVE_SYNC_COMPARE_AND_SWAP_16 ) ||                        \
    ( (__GNUC__ >  4) || ( (__GNUC__ >= 4) && (__GNUC_MINOR__ >= 2) ) && defined(__nocona__ ))

__forceinline bool WINAPI CompareAndSwap128(
	volatile LONG64 addr[2], LONG64 old1, LONG64 old2, LONG64 new1, LONG64 new2)
{
	typedef int ATOMIC_LONG128_ __attribute__ ((mode (TI)));
	typedef ATOMIC_LONG128_ ATOMIC_LONG128;
	typedef ATOMIC_LONG128_* PATOMIC_LONG128;

	return __sync_bool_compare_and_swap_16(
		(volatile PATOMIC_LONG128)addr,
		old1 | ((ATOMIC_LONG128)old2 << 64),
		new1 | ((ATOMIC_LONG128)new2 << 64));	
}

#elif defined(__GNUC__) && defined(__x86_64__)

__forceinline bool WINAPI CompareAndSwap128(
	volatile LONG64 addr[2], LONG64 old1, LONG64 old2, LONG64 new1, LONG64 new2)
{
    /* handcoded asm, will crash on early amd processors */
    char result;
    __asm__ __volatile__("lock; cmpxchg16b %0; setz %1"
                         : "=m"(*addr), "=q"(result)
                         : "m"(*addr), "d" (old2), "a" (old1),
                           "c" (new2), "b" (new1) : "memory");
    return (bool)result;
}

#else

#define BOOST_DETAIL_NO_COMPARE_AND_SWAP_128

#endif

// -------------------------------------------------------------------------
// TaggedCompareAndSwap

__forceinline bool WINAPI TaggedCompareAndSwap(
	volatile LONG32 Destination[2], LONG32 Comperand, LONG32 Exchange, LONG32 Tag)
{
	return __sync_bool_compare_and_swap_8(
		(volatile PLONG64)Destination,
		Comperand | ((LONG64)Tag << 32),
		Exchange | ((LONG64)(Tag+1) << 32));
}

#if !defined(BOOST_DETAIL_NO_COMPARE_AND_SWAP_128)

__forceinline bool WINAPI TaggedCompareAndSwap(
	volatile LONG64 Destination[2], LONG64 Comperand, LONG64 Exchange, LONG64 Tag)
{
	return CompareAndSwap128(Destination, Comperand, Tag, Exchange, Tag+1);
}

#endif

#if defined(__32BIT__) || defined(__x86_32__)

template <class Type>
__forceinline bool WINAPI TaggedCompareAndSwapPointer(
	Type* Destination[2], Type* Comperand, Type* Exchange, Type* Tag)
{
	return TaggedCompareAndSwap(
		(volatile LONG32*)Destination, (LONG32)Comperand, (LONG32)Exchange, (LONG32)Tag);
}

#elif defined(__64BIT__) || defined(__x86_64__)

template <class Type>
__forceinline bool WINAPI TaggedCompareAndSwapPointer(
	Type* Destination[2], Type* Comperand, Type* Exchange, Type* Tag)
{
	return TaggedCompareAndSwap(
		(volatile LONG64*)Destination, (LONG64)Comperand, (LONG64)Exchange, (LONG64)Tag);
}

#else

#define BOOST_DETAIL_NO_TAGGED_COMPARE_AND_SWAP_POINTER

#endif

// -------------------------------------------------------------------------
// $Log: $

#endif /* BOOST_DETAIL_WINAPI_ATOMIC_ATOMIC_BUILTINS_HPP */

