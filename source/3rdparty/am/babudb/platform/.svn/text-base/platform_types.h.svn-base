// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#ifndef YIELD_PLATFORM_PLATFORM_TYPES_H
#define YIELD_PLATFORM_PLATFORM_TYPES_H

#include "yield/platform/platform_config.h" // for YIELD

#include <stdint.h>
#include <sys/types.h> // For size_t

namespace YIELD
{
	typedef uint32_t timeout_ns_t; // This is a uint32_t (which can only store ~4.3s in ns) because Solaris+gcc (4.0, 4.2) mangles uint64_t's on function calls in 32-bit binaries (no explanation for that yet)

#ifdef _WIN32
	typedef void* fd_t;
	typedef unsigned int socket_t;

	struct iovec_t
	{
		size_t len;
		void* buf;
	};

	typedef __int64 ssize_t;

	#ifndef MAX_PATH
	#define MAX_PATH 260
	#endif
#else
	typedef int fd_t;
	typedef int socket_t;
	#define INVALID_HANDLE_VALUE -1

	struct iovec_t
	{
		void* buf;
		size_t len;
	};

	#ifdef __MACH__
	#define MAX_PATH PATH_MAX
	#else
	#define MAX_PATH 512
	#endif
#endif

	inline uint32_t upper32( uint64_t val ) { return ( uint32_t )( val >> 32 ); }
	inline uint32_t lower32( uint64_t val ) { return ( uint32_t )( val & 0xffFFffFF ); }
	inline uint64_t create_uint64( uint32_t upper, uint32_t lower ) { return ( ( uint64_t )upper ) << 32 | ( uint64_t )lower; }
};

#ifndef SIZE_MAX
#define SIZE_MAX ( ( size_t ) - 1 )
#endif

#endif
