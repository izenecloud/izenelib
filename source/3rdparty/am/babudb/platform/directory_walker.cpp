// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#include "platform/directory_walker.h"
#include "platform/platform_exception.h"
#include "platform/debug.h"
using namespace YIELD;

using namespace std;

#ifdef _WIN32
#define UNICODE
#include "yield/platform/windows.h"
inline uint64_t WIN2UNIX( uint64_t time ) { return ( ( time - ( uint64_t )116444736000000000LL ) / ( uint64_t )10000000LL ); }
#else
#include <dirent.h>
#endif


DirectoryWalker::DirectoryWalker( const Path& root_dir_path )
	: root_dir_path( root_dir_path )
#ifdef _WIN32
	, search_pattern( root_dir_path.getWidePath() )
#endif
{
	init();
}

DirectoryWalker::DirectoryWalker( const Path& root_dir_path, const Path& match_file_name_prefix  )
	: root_dir_path( root_dir_path ), match_file_name_prefix( match_file_name_prefix )
#ifdef _WIN32
	, search_pattern( root_dir_path.getWidePath() )
#endif
{
	init();
}

void DirectoryWalker::init()
{
#ifdef _WIN32
	if ( search_pattern.size() > 0 && search_pattern[search_pattern.size()-1] != L'\\' ) search_pattern.append( L"\\" );
	search_pattern.append( match_file_name_prefix.getWidePath() ).append( L"*" );
	scan_handle = NULL;
#else
	scan_handle = opendir( root_dir_path.getHostCharsetPath().c_str() );
#endif
}

DirectoryWalker::~DirectoryWalker()
{
	if ( scan_handle )
#ifdef _WIN32
		FindClose( scan_handle );
#else
		closedir( ( DIR* )scan_handle );
#endif
}

bool DirectoryWalker::hasNext()
{
#ifdef _WIN32
	WIN32_FIND_DATA next_find_data;

	while ( true )
	{
		if ( scan_handle != NULL )
		{
			if ( !FindNextFileW( scan_handle, &next_find_data ) )
				return false;
		}
		else if ( ( scan_handle = FindFirstFileW( search_pattern.c_str(), &next_find_data ) ) == INVALID_HANDLE_VALUE )
			return false;

		if ( next_find_data.cFileName[0] == '.' )
			continue;

		wstring next_path( root_dir_path.getWidePath() );
		if ( next_path.size() > 0 && next_path[next_path.size()-1] != L'\\' ) next_path += L'\\';
		next_path += next_find_data.cFileName;
		if ( ( next_find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 ) next_path += ( L'\\' );

		try
		{
			next_directory_entry.reset(
				new DirectoryEntry( next_path,
									( next_find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ? Stat::Directory : Stat::File,
								    ( size_t )create_uint64( next_find_data.nFileSizeHigh, next_find_data.nFileSizeLow ),
									WIN2UNIX( create_uint64( next_find_data.ftLastWriteTime.dwHighDateTime, next_find_data.ftLastWriteTime.dwLowDateTime ) ),
									WIN2UNIX( create_uint64( next_find_data.ftCreationTime.dwHighDateTime, next_find_data.ftCreationTime.dwLowDateTime ) ),
									WIN2UNIX( create_uint64( next_find_data.ftLastAccessTime.dwHighDateTime, next_find_data.ftLastAccessTime.dwLowDateTime ) ),
									false ) );

			return true;
		}
		catch ( exception& )
		{ }
	}
#elif !defined(__sun)
	struct dirent* next_dirent = NULL;
        if (scan_handle == NULL) {
          return false;
        }

	while ( ( next_dirent = readdir( ( DIR* )scan_handle ) ) )
	{
		if ( next_dirent->d_name[0] != '.' && ( next_dirent->d_type == DT_DIR || next_dirent->d_type == DT_REG ) )
		{
			if ( !match_file_name_prefix.getHostCharsetPath().empty() && 
				 strstr( next_dirent->d_name, match_file_name_prefix.getHostCharsetPath().c_str() ) != next_dirent->d_name )
				continue;

			string next_path( root_dir_path.getHostCharsetPath() );
			if ( next_path.size() > 0 && next_path[next_path.size()-1] != '/' ) next_path.append( "/", 1 );
			next_path.append( next_dirent->d_name );
			if ( next_dirent->d_type == DT_DIR ) next_path += '/';

			try
			{
				next_directory_entry.reset( new DirectoryEntry( next_path ) );
				return true;
			}
			catch ( Exception& )
			{ }
		}
	}

	return false;
#else
	DebugBreak();
	return false;
#endif
}
