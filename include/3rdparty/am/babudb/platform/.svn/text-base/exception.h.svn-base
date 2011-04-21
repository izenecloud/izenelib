// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#ifndef YIELD_PLATFORM_EXCEPTION_H
#define YIELD_PLATFORM_EXCEPTION_H

#include "yield/platform/platform_config.h" // for YIELD

#include <exception>


namespace YIELD
{
	class Exception : public std::exception
	{
	public:
		Exception() : _what( "" ) { }
		Exception( const char* what ) : _what( what ) { }
		virtual ~Exception() throw() { }

		// exception
		virtual const char* what() const throw() { return _what; }

	private:
		const char* _what;
	};
};

#endif
