// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#ifndef YIELD_PLATFORM_ASSERT_H
#define YIELD_PLATFORM_ASSERT_H

#include "yield/platform/exception.h"

#ifdef _WIN32
#include <stdio.h>
#else
#include <cstdio>
#endif


namespace YIELD
{
	class AssertionException : public Exception
	{
	public:
		AssertionException( const char* file_name, int line_number, const char* info = "" )
		{
#ifdef _WIN32
			_snprintf_s( what_buffer, 1024, "line number %d in %s (%s)", line_number, file_name, info );
#else
			std::snprintf( what_buffer, 1024, "line number %d in %s (%s)", line_number, file_name, info );
#endif
		}

		virtual const char* what() const throw() { return what_buffer; }

	private:
		char what_buffer[1024];
	};
};


#define FAIL() throw YIELD::AssertionException( __FILE__, __LINE__ );
#define ASSERT_TRUE( stat ) { if ( !( ( stat ) == true ) ) throw YIELD::AssertionException( __FILE__, __LINE__, #stat" != true" ); }
#define ASSERT_FALSE( stat ) { if ( !( ( stat ) == false ) ) throw YIELD::AssertionException( __FILE__, __LINE__, #stat" != false" ); }
#define ASSERT_EQUAL( stat_a, stat_b ) { if ( !( ( stat_a ) == ( stat_b ) ) ) throw YIELD::AssertionException( __FILE__, __LINE__, #stat_a" != "#stat_b ); }
#define ASSERT_NOTEQUAL( stat_a, stat_b ) { if ( !( ( stat_a ) != ( stat_b ) ) ) throw YIELD::AssertionException( __FILE__, __LINE__, #stat_a" == "#stat_b ); }

#endif
