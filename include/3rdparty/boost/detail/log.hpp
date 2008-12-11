//
//  boost/detail/log.hpp
//
//  Copyright (c) 2004 - 2008 xushiwei (xushiweizh@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/detail/todo.htm for documentation.
//
#ifndef BOOST_DETAIL_LOG_HPP
#define BOOST_DETAIL_LOG_HPP

#if !defined(_CSTDIO_) && !defined(_CSTDIO)
#include <cstdio> // vprintf, vfprintf
#endif

#if !defined(_CSTDARG_) && !defined(_CSTDARG)
#include <cstdarg> // va_list
#endif

// -------------------------------------------------------------------------

#ifndef NS_BOOST_DETAIL_BEGIN
#define NS_BOOST_DETAIL_BEGIN	namespace boost { namespace detail {
#define NS_BOOST_DETAIL_END		} }
#define NS_BOOST_DETAIL			boost::detail
#endif

#ifndef BOOST_DETAIL_CALL
#define BOOST_DETAIL_CALL
#endif

NS_BOOST_DETAIL_BEGIN

// -------------------------------------------------------------------------
// class null_log

class null_log
{
public:
	null_log& BOOST_DETAIL_CALL trace(const char* fmt, ...)
	{
		return *this;
	}
};

// -------------------------------------------------------------------------
// class stdout_log

class stdout_log
{
public:
	stdout_log& BOOST_DETAIL_CALL trace(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
		return *this;
	}
};

// -------------------------------------------------------------------------
// class stderr_log

class stderr_log
{
public:
	stderr_log& BOOST_DETAIL_CALL trace(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		vfprintf(stderr, fmt, args);
		va_end(args);
		return *this;
	}
};

// -------------------------------------------------------------------------
// $Log: $

NS_BOOST_DETAIL_END

#endif /* BOOST_DETAIL_LOG_HPP */
