// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#include "platform/platform_exception.h"
using namespace YIELD;

#ifdef _WIN32
#include "platform/windows.h"
#include <cstdio>
#else
#include <errno.h>
#include <cstdio>
#include <cstring>
using std::strncpy;
#ifdef __sun
#include <stdio.h> // For snprintf
#endif
#endif


void PlatformException::strerror( unsigned long error_code, char* error_code_str, unsigned int error_code_str_len )
{
#ifdef _WIN32
	DWORD dwStrLen = FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error_code, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), error_code_str, error_code_str_len, NULL );
	if ( dwStrLen > 2 ) error_code_str[dwStrLen-2] = 0; // Cut off trailing \r\n
#else
	strncpy( error_code_str, std::strerror( error_code ), error_code_str_len );
#endif
}

PlatformException::PlatformException()
{
#ifdef _WIN32
		error_code = ::GetLastError();
#else
		error_code = ( unsigned long )errno;
#endif
}

PlatformException::PlatformException( unsigned long error_code ) : error_code( error_code )
{ }

const char* PlatformException::what() const throw()
{
#ifdef _WIN32
	int written = _snprintf_s( ( char* )&what_buffer, 100, _TRUNCATE, "PlatformException: %lu ", error_code );
#else
	int written = snprintf( ( char* )&what_buffer, 100, "PlatformException: %lu ", error_code );
#endif

	if ( written > 0 && written < 100 )
		PlatformException::strerror( error_code, ( char* )&what_buffer[written], 100-written-1 );

	return what_buffer;
}
