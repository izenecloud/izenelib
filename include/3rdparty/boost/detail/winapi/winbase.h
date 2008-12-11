//
//  boost/detail/winapi/winbase.h
//
//  Copyright (c) 2004 - 2008 xushiwei (xushiweizh@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/detail/todo.htm for documentation.
//
#ifndef BOOST_DETAIL_WINAPI_WINBASE_H
#define BOOST_DETAIL_WINAPI_WINBASE_H

#if defined(_WIN32) || defined(_WIN64)

#ifndef __wtypes_h__
#include <wtypes.h>
#endif

#ifndef _WINBASE_
#include <winbase.h>
#endif

#ifndef _BASETSD_H_
#include <basetsd.h>
#endif

#if !defined(_W64)
	#define BOOST_DETAIL_WINSDK_VC6
	#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300
	#define _W64 __w64
	#else
	#define _W64
	#endif
#endif

#if defined(BOOST_DETAIL_WINSDK_VC6)
	#if defined(_WIN64)
		typedef __int64 LONG_PTR, *PLONG_PTR;
		typedef unsigned __int64 ULONG_PTR, *PULONG_PTR;
	#else
		typedef _W64 long LONG_PTR, *PLONG_PTR;
		typedef _W64 unsigned long ULONG_PTR, *PULONG_PTR;
	#endif
#endif

#ifndef BOOST_DETAIL_WINAPI_ATOMIC_WINDOWS_HPP
#include "atomic/windows.hpp"
#endif

#else

#define BOOST_DETAIL_NO_WINSDK

#ifndef BOOST_DETAIL_WINAPI_WTYPES_H
#include "wtypes.h"
#endif

#ifndef BOOST_DETAIL_WINAPI_POSIX_PTHREAD_HPP
#include "posix/pthread.hpp"
#endif

#ifndef BOOST_DETAIL_WINAPI_POSIX_TIME_HPP
#include "posix/time.hpp"
#endif

#ifndef BOOST_DETAIL_WINAPI_ATOMIC_ATOMIC_BUILTINS_HPP
#include "atomic/atomic_builtins.hpp"
#endif

#endif

// -------------------------------------------------------------------------
// $Log: $

#endif /* BOOST_DETAIL_WINAPI_WINBASE_H */
