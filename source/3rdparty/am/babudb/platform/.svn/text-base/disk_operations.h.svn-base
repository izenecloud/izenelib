// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#ifndef YIELD_PLATFORM_DISK_OPERATIONS_H
#define YIELD_PLATFORM_DISK_OPERATIONS_H

#include "yield/platform/platform_types.h"
#include "yield/platform/path.h"

#include <fcntl.h>

#ifdef _WIN32
#define O_SYNC 010000
#define O_FSYNC 010000
#define O_ASYNC 020000
#define O_DIRECT 040000
#define O_HIDDEN 0100000
#endif

#define O_THROW_EXCEPTIONS 04000000
#define O_CLOSE_ON_DESTRUCT 010000000
#define O_SPARSE 020000000


namespace YIELD
{
	class DiskOperations
	{
	public:
		static fd_t open( const Path&, unsigned long flags = O_RDONLY );
		static bool close( fd_t, unsigned long flags = 0 );
		static bool exists( const Path& );
		static bool touch( const Path&, unsigned long flags = 0 );
		static bool unlink( const Path&, unsigned long flags = 0 );
		static bool rename( const Path& from_path, const Path& to_path , unsigned long flags = 0 );
		static bool mkdir( const Path&, unsigned long flags = 0 );
		static bool mktree( const Path&, unsigned long flags = 0 );
		static bool makedirs( const Path& path, unsigned long flags = 0 ) { return mktree( path ); } // Python function name
		static bool rmdir( const Path&, unsigned long flags = 0 );
		static bool rmtree( const Path&, unsigned long flags = 0 );

	private:
		static bool _rmtree( const Path&, unsigned long flags  );
	};
};

#endif
