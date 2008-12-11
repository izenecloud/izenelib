//
//  boost/detail/winapi/posix/time.hpp
//
//  Copyright (c) 2004 - 2008 xushiwei (xushiweizh@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/detail/todo.htm for documentation.
//
#ifndef BOOST_DETAIL_WINAPI_POSIX_TIME_HPP
#define BOOST_DETAIL_WINAPI_POSIX_TIME_HPP

#ifndef BOOST_DETAIL_WINAPI_WINDEF_H
#include "../windef.h"
#endif

// -------------------------------------------------------------------------
// int clock_getres(clockid_t clk_id, struct timespec *res);
// int clock_gettime(clockid_t clk_id, struct timespec *tp);
// int clock_settime(clockid_t clk_id, const struct timespec *tp);

#ifndef _TIME_H
#include <time.h>
#endif

#ifndef _UNISTD_H
#include <unistd.h>
#endif

// See http://linux.die.net/man/3/clock_gettime
// On POSIX systems on which these functions are available, the symbol _POSIX_TIMERS
// is defined in <unistd.h> to a value greater than 0.
// The symbols _POSIX_MONOTONIC_CLOCK, _POSIX_CPUTIME, _POSIX_THREAD_CPUTIME indicate 
// that CLOCK_MONOTONIC, CLOCK_PROCESS_CPUTIME_ID, CLOCK_THREAD_CPUTIME_ID are available.
#if defined(_POSIX_TIMERS) && (_POSIX_TIMERS > 0) && defined(_POSIX_MONOTONIC_CLOCK)
#define BOOST_DETAIL_HAS_MONOTONIC_CLOCK
#endif

// -------------------------------------------------------------------------

#if defined(BOOST_DETAIL_HAS_MONOTONIC_CLOCK)

__forceinline BOOL WINAPI QueryPerformanceFrequency(LARGE_INTEGER* lp)
{
	lp->QuadPart = 1000000000;
	return TRUE;
}

__forceinline BOOL WINAPI QueryPerformanceCounter(LARGE_INTEGER* lp)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    UINT64 ticks = (UINT64)now.tv_sec * 1000000000 + now.tv_nsec;
    lp->QuadPart = ticks;
	return TRUE;
}

#endif

// -------------------------------------------------------------------------
// $Log: $

#endif /* BOOST_DETAIL_WINAPI_POSIX_TIME_HPP */
