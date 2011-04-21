// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#ifndef YIELD_PLATFORM_PLATFORM_EXCEPTION_H
#define YIELD_PLATFORM_PLATFORM_EXCEPTION_H

#include "yield/platform/exception.h"


namespace YIELD
{
	class PlatformException : public Exception
	{
	public:
		static void strerror( unsigned long error_code, char* error_code_str, unsigned int error_code_str_len );

		PlatformException(); // Get error code from errno or GetLastError()
		PlatformException( unsigned long error_code );

		// exception
		virtual const char* what() const throw();

	private:
		unsigned long error_code;
		char what_buffer[100];
	};
};

#endif
