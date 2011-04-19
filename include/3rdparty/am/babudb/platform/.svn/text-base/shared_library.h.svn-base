// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#ifndef YIELD_PLATFORM_SHARED_LIBRARY_H
#define YIELD_PLATFORM_SHARED_LIBRARY_H

#include "yield/platform/platform_types.h"
#include "yield/platform/path.h"

#if defined(_MSC_VER)
#define DLLEXPORT extern "C" __declspec(dllexport)
#elif defined(YIELD_HAVE_GCCVISIBILITIYPATCH) || ( defined(__GNUC__) && __GNUC__ >= 4 )
#define DLLEXPORT extern "C" __attribute__ ((visibility("default")))
#else
//#error
#define DLLEXPORT extern "C"
#endif


namespace YIELD
{
	class SharedLibrary
	{
	public:
		SharedLibrary( const Path& path, const char* argv0 = 0 );
		~SharedLibrary();

		const Path& getFilePrefix() const { return file_prefix; }
		void* getHandle() { return handle; }
		void* getFunction( const char* function_name );

	private:
		Path file_prefix;
		void* handle;
	};
};

#endif
