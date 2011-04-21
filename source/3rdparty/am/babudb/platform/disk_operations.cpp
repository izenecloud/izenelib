// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#include "platform/disk_operations.h"
#include "platform/directory_walker.h"
#include "platform/platform_exception.h"
using namespace YIELD;

using namespace std;

#ifdef _WIN32

#define UNICODE
#include "yield/platform/windows.h"
typedef struct _FILE_FULL_EA_INFORMATION {
    ULONG NextEntryOffset;
    UCHAR Flags;
    UCHAR EaNameLength;
    USHORT EaValueLength;
    CHAR EaName[1];
} FILE_FULL_EA_INFORMATION, *PFILE_FULL_EA_INFORMATION;

#else

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>

#endif


fd_t DiskOperations::open( const Path& path, unsigned long flags )
{
#ifdef _WIN32
	DWORD file_access_flags = 0,
		  file_share_flags = FILE_SHARE_READ,
		  file_create_flags = 0,
		  file_open_flags = FILE_FLAG_SEQUENTIAL_SCAN;

	if ( ( flags & O_RDONLY ) == O_RDONLY || ( flags & O_RDWR ) == O_RDWR )
		file_access_flags |= GENERIC_READ;

	if ( ( flags & O_WRONLY ) == O_WRONLY || ( flags & O_RDWR ) == O_RDWR )
		file_access_flags |= GENERIC_WRITE;
	else if ( ( flags & O_APPEND ) == O_APPEND )
		file_access_flags |= FILE_APPEND_DATA;

	if ( file_access_flags == 0 )
		file_access_flags = GENERIC_READ|GENERIC_WRITE;

	if ( ( flags & O_CREAT ) == O_CREAT )
		file_create_flags = OPEN_ALWAYS;
	else if ( ( flags & O_TRUNC ) == O_TRUNC )
		file_create_flags = CREATE_ALWAYS;
	else
		file_create_flags = OPEN_EXISTING;

	if ( ( flags & O_SPARSE ) == O_SPARSE )
		file_open_flags |= FILE_ATTRIBUTE_SPARSE_FILE;

	if ( ( flags & O_SYNC ) == O_SYNC )
		file_open_flags |= FILE_FLAG_WRITE_THROUGH;

	if ( ( flags & O_DIRECT ) == O_DIRECT )
		file_open_flags |= FILE_FLAG_NO_BUFFERING;

	if ( ( flags & O_ASYNC ) == O_ASYNC )
		file_open_flags |= FILE_FLAG_OVERLAPPED;

	if ( ( flags & O_HIDDEN ) == O_HIDDEN )
		file_open_flags = FILE_ATTRIBUTE_HIDDEN;

	HANDLE fd = CreateFileW( path.getWidePath().c_str(), file_access_flags, file_share_flags, NULL, file_create_flags, file_open_flags, NULL );
#else
	int fd = ::open( path.getHostCharsetPath().c_str(), flags, S_IRUSR|S_IWUSR );
#endif


	if ( fd == INVALID_HANDLE_VALUE )
	{
		if ( ( flags & O_THROW_EXCEPTIONS ) == O_THROW_EXCEPTIONS )
			throw PlatformException();
		else
			return INVALID_HANDLE_VALUE;
	}

	return fd;
}

bool DiskOperations::close( fd_t fd, unsigned long flags )
{
#ifdef _WIN32
	if ( CloseHandle( fd ) )
#else
	if ( ::close( fd ) >= 0 )
#endif
		return true;
	else if ( ( flags & O_THROW_EXCEPTIONS ) == O_THROW_EXCEPTIONS )
		throw PlatformException();
	else
		return false;
}

bool DiskOperations::exists( const Path& path )
{
#ifdef _WIN32
	return GetFileAttributesW( path.getWidePath().c_str() ) != INVALID_FILE_ATTRIBUTES;
#else
	struct stat temp_stat;
	return ::stat( path.getHostCharsetPath().c_str(), &temp_stat ) == 0;
#endif
}

bool DiskOperations::touch( const Path& path, unsigned long flags )
{
	fd_t fd = DiskOperations::open( path, O_CREAT|O_TRUNC|O_WRONLY|flags );
	if ( fd != INVALID_HANDLE_VALUE )
	{
		DiskOperations::close( fd, flags );
		return true;
	}
	else
		return false;
}

bool DiskOperations::unlink( const Path& path, unsigned long flags )
{
#ifdef _WIN32
	if ( DeleteFileW( path.getWidePath().c_str() ) )
#else
	if ( ::unlink( path.getHostCharsetPath().c_str() ) >= 0 )
#endif
		return true;
	else if ( ( flags & O_THROW_EXCEPTIONS ) == O_THROW_EXCEPTIONS )
		throw PlatformException();
	else
		return false;
}

bool DiskOperations::rename( const Path& from_path, const Path& to_path, unsigned long flags )
{
#ifdef _WIN32
	if ( MoveFileExW( from_path.getWidePath().c_str(), to_path.getWidePath().c_str(), MOVEFILE_REPLACE_EXISTING ) )
#else
	if ( ::rename( from_path.getHostCharsetPath().c_str(), to_path.getHostCharsetPath().c_str() ) >= 0 )
#endif
		return true;
	else if ( ( flags & O_THROW_EXCEPTIONS ) == O_THROW_EXCEPTIONS )
		throw PlatformException();
	else
		return false;
}

bool DiskOperations::mkdir( const Path& path, unsigned long flags )
{
#ifdef _WIN32
	if ( CreateDirectoryW( path.getWidePath().c_str(), NULL ) )
#else
	if ( ::mkdir( path.getHostCharsetPath().c_str(), S_IRWXU ) >= 0 )
#endif
		return true;
	else if ( ( flags & O_THROW_EXCEPTIONS ) == O_THROW_EXCEPTIONS )
		throw PlatformException();
	else
		return false;
}

bool DiskOperations::mktree( const Path& path, unsigned long flags )
{
	bool ret = true;

	pair<Path, Path> path_parts = path.split();
	if ( !path_parts.first.getHostCharsetPath().empty() )
		ret &= mktree( path_parts.first, flags );

	if ( !exists( path ) && !mkdir( path, flags ) )
	{
		if ( ( flags & O_THROW_EXCEPTIONS ) == O_THROW_EXCEPTIONS )
			throw PlatformException();
		else
			return false;
	}

	return ret;
}

bool DiskOperations::rmdir( const Path& path, unsigned long flags )
{
#ifdef _WIN32
	if ( RemoveDirectoryW( path.getWidePath().c_str() ) )
#else
	if ( ::rmdir( path.getHostCharsetPath().c_str() ) >= 0 )
#endif
		return true;
	else if ( ( flags & O_THROW_EXCEPTIONS ) == O_THROW_EXCEPTIONS )
		throw PlatformException();
	else
		return false;
}

bool DiskOperations::rmtree( const Path& path, unsigned long flags )
{
	Stat path_stat( path );
	if ( path_stat.getType() == Stat::Directory )
		return _rmtree( path, flags );
	else // if ( path_stat.getType() == Stat::File )
		return unlink( path, flags );
}

bool DiskOperations::_rmtree( const Path& path, unsigned long flags )
{
	{ // Locking scope for the walker, so it releases its lock on the directory before rmdir below
		DirectoryWalker walker( path );
		while ( walker.hasNext() )
		{
			auto_ptr<DirectoryEntry> entry = walker.getNext();
			if ( entry->getType() == Stat::Directory )
			{
				if ( _rmtree( entry->getPath(), flags ) )
					continue;
				else
					return false;
			}
			else if ( entry->getType() == Stat::File )
			{
				if ( unlink( entry->getPath(), flags ) )
					continue;
				else
					return false;
			}
		}
	}

	return rmdir( path, flags );
}
