//
//  boost/detail/winapi/posix/pthread.hpp
//
//  Copyright (c) 2004 - 2008 xushiwei (xushiweizh@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/detail/todo.htm for documentation.
//
#ifndef BOOST_DETAIL_WINAPI_POSIX_PTHREAD_HPP
#define BOOST_DETAIL_WINAPI_POSIX_PTHREAD_HPP

#ifndef BOOST_DETAIL_WINAPI_WINDEF_H
#include "../windef.h"
#endif

#ifndef _PTHREAD_H
#include <pthread.h>
#endif

// -------------------------------------------------------------------------
// CriticalSection

typedef pthread_mutex_t RTL_CRITICAL_SECTION, *PRTL_CRITICAL_SECTION;
typedef RTL_CRITICAL_SECTION CRITICAL_SECTION;
typedef PRTL_CRITICAL_SECTION PCRITICAL_SECTION;
typedef PRTL_CRITICAL_SECTION LPCRITICAL_SECTION;

__forceinline VOID WINAPI InitializeCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection)
{
	pthread_mutex_init(lpCriticalSection, NULL);
}

__forceinline VOID WINAPI EnterCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection)
{
	pthread_mutex_lock(lpCriticalSection);
}

__forceinline BOOL WINAPI TryEnterCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection)
{
	return pthread_mutex_trylock(lpCriticalSection) == 0;
}

__forceinline VOID WINAPI LeaveCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection)
{
	pthread_mutex_unlock(lpCriticalSection);
}

__forceinline VOID WINAPI DeleteCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection)
{
	pthread_mutex_destroy(lpCriticalSection);
}

// -------------------------------------------------------------------------

__forceinline DWORD WINAPI GetCurrentThreadId()
{
#if defined(PTW32_VERSION)
	return (DWORD)pthread_self().p;
#else 
#ifdef __APPLE__
	return (DWORD)(pthread_self()->__sig);
#else
	return pthread_self();
#endif
#endif
}

// -------------------------------------------------------------------------

#ifndef TLS_OUT_OF_INDEXES
#define TLS_OUT_OF_INDEXES (DWORD)0xFFFFFFFF
#endif

#ifndef S_OK
#define S_OK 0
#endif

typedef pthread_key_t TLSINDEX;

__forceinline TLSINDEX WINAPI TlsAlloc(void)
{
	pthread_key_t key;
	if (pthread_key_create(&key, NULL) != S_OK)
		return TLS_OUT_OF_INDEXES;
	else
		return key;
}

__forceinline BOOL WINAPI TlsFree(TLSINDEX key)
{
	return pthread_key_delete(key) == S_OK;
}

__forceinline BOOL WINAPI TlsSetValue(TLSINDEX key, LPVOID lpTlsValue)
{
	return pthread_setspecific(key, lpTlsValue) == S_OK;
}

__forceinline LPVOID WINAPI TlsGetValue(TLSINDEX key)
{
	return pthread_getspecific(key);
}

// -------------------------------------------------------------------------
// $Log: $

#endif /* BOOST_DETAIL_WINAPI_POSIX_PTHREAD_HPP */
