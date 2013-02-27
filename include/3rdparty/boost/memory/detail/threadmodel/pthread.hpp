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

#include <pthread.h>

// -------------------------------------------------------------------------

inline unsigned long GetCurrentThreadId()
{
    return pthread_self();
}

// -------------------------------------------------------------------------

#ifndef TLS_OUT_OF_INDEXES
#define TLS_OUT_OF_INDEXES (unsigned long)0xFFFFFFFF
#endif

#ifndef S_OK
#define S_OK 0
#endif

typedef pthread_key_t TLSINDEX;

inline TLSINDEX TlsAlloc(void)
{
    pthread_key_t key;
    if (pthread_key_create(&key, NULL) != S_OK)
        return TLS_OUT_OF_INDEXES;
    else
        return key;
}

inline bool TlsFree(TLSINDEX key)
{
    return pthread_key_delete(key) == S_OK;
}

inline bool TlsSetValue(TLSINDEX key, void * lpTlsValue)
{
    return pthread_setspecific(key, lpTlsValue) == S_OK;
}

inline void * TlsGetValue(TLSINDEX key)
{
    return pthread_getspecific(key);
}

// -------------------------------------------------------------------------
// $Log: $

#endif /* BOOST_DETAIL_WINAPI_POSIX_PTHREAD_HPP */
