// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#ifndef YIELD_PLATFORM_DIRECTORY_WALKER_H
#define YIELD_PLATFORM_DIRECTORY_WALKER_H

#include "yield/platform/stat.h"

#include <memory>


namespace YIELD
{
	class DirectoryEntry : public Stat
	{
	public:
		const Path& getPath() { return path; }

	private:
		friend class DirectoryWalker;

		DirectoryEntry( const Path& path )
			: Stat( path ), path( path )
		{ }

		DirectoryEntry( const Path& path, StatType type, size_t size, int64_t last_write_time, int64_t creation_time, int64_t last_access_time, bool is_hidden )
			: Stat( type, size, last_write_time, creation_time, last_access_time, is_hidden ), path( path )
		{ }

		Path path;
	};


	class DirectoryWalker
	{
	public:
		DirectoryWalker( const Path& root_dir_path );
		DirectoryWalker( const Path& root_dir_path, const Path& match_file_name_prefix );
		~DirectoryWalker();

		bool hasNext();
		std::auto_ptr<DirectoryEntry> getNext() { return next_directory_entry; }

	private:
		void init();

		Path root_dir_path, match_file_name_prefix;

		std::auto_ptr<DirectoryEntry> next_directory_entry;

		void* scan_handle;
#ifdef _WIN32
		std::wstring search_pattern;
#endif
	};
};

#endif
