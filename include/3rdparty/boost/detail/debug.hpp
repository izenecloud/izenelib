//
//  boost/detail/debug.hpp
//
//  Copyright (c) 2004 - 2008 xushiwei (xushiweizh@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/detail/todo.htm for documentation.
//
#ifndef BOOST_DETAIL_DEBUG_HPP
#define BOOST_DETAIL_DEBUG_HPP

#if defined(_MSC_VER) && !defined(_INC_CRTDBG)
#include <crtdbg.h> // _CrtSetDbgFlag, _ASSERTE
#endif

// -------------------------------------------------------------------------
// BOOST_DETAIL_ASSERT - diagnost

#ifndef BOOST_DETAIL_ASSERT
	#if defined(ASSERT)
		#define BOOST_DETAIL_ASSERT(e)		ASSERT(e)
	#elif defined(_ASSERTE)
		#define BOOST_DETAIL_ASSERT(e)		_ASSERTE(e)
	#elif defined(BOOST_ASSERT)
		#define BOOST_DETAIL_ASSERT(e)		BOOST_ASSERT(e)
	#else
		#ifdef _DEBUG
		#ifndef assert
		#include <cassert>
		#endif
		#define BOOST_DETAIL_ASSERT(e)		assert(e)
		#else
		#define BOOST_DETAIL_ASSERT(e)
		#endif
	#endif
#endif

// -------------------------------------------------------------------------
// $Log: $

#endif /* BOOST_DETAIL_DEBUG_HPP */
