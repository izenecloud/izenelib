// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#ifndef YIELD_PLATFORM_STAT_H
#define YIELD_PLATFORM_STAT_H

#include "yield/platform/platform_types.h"
#include "yield/platform/path.h"


namespace YIELD
{
	class Stat
	{
	public:
		enum StatType { Volume, File, Directory };

		Stat( const Path& path ) { init( path ); }
#ifdef _WIN32
		// Since fd_t is void* on Windows we need these to delegate to the const Path& variant rather than the fd_t one
		Stat( char* path ) { init( Path( path ) ); }
		Stat( const char* path ) { init( Path( path ) ); }
#endif
		Stat( fd_t fd ) { init( fd ); }
		Stat( StatType type, size_t _st_size, int64_t _st_mtime, int64_t _st_ctime, int64_t _st_atime, bool is_hidden = false );
		Stat( const Stat& );

		StatType getType() { return type; }
		size_t getSize() const { return _st_size; }
		int64_t getLastWriteTime() const { return _st_mtime; }
		int64_t getCreationTime() const { return _st_ctime; }
		int64_t getLastAccessTime() const { return _st_atime; }
		bool isHidden() const { return is_hidden; }

	protected:
		void init( const Path& );  // Opens path, fstats the file descriptor, and closes it
		void init( fd_t ); // fstats the file descriptor but doesn't close it

		StatType type;
		size_t _st_size; int64_t _st_mtime, _st_ctime, _st_atime; // Unix doesn't like using the actual names, thus the _
		bool is_hidden;
	};
};

#endif
