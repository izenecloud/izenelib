// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#include "platform/memory_mapped_file.h"
using namespace YIELD;

#include "platform/platform_exception.h"
#include "platform/debug.h"

#ifdef _WIN32
#define UNICODE
#include "platform/windows.h"
#else
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#endif

#include <algorithm>
using std::min;
using std::max;


MemoryMappedFile::MemoryMappedFile( const Path& path, size_t minimum_region_size, unsigned long flags ) 
	: File( path, flags )
{
	if ( ( flags & O_TRUNC ) == 0 )
	{
#ifdef _WIN32
		unsigned int size_low, size_high;
		size_low = GetFileSize( fd, ( LPDWORD ) & size_high );
		size = ( size_t )create_uint64( size_high, size_low );
#else
		struct stat temp_stat;
		if ( stat( path.getHostCharsetPath().c_str(), &temp_stat ) != -1 )
			size = temp_stat.st_size;
		else
			size = 0;
#endif
	}
	else
		size = 0;

	start = NULL;
#ifdef _WIN32
	mapping = NULL;
#endif

	resize( max( minimum_region_size, size ) );
}

bool MemoryMappedFile::close()
{
	if ( start != NULL )
	{
		writeBack();

#ifdef _WIN32
		UnmapViewOfFile( start );
#else
		munmap( start, size );
#endif
		start = NULL;
	}

#ifdef _WIN32
	if ( mapping != NULL )
	{
		CloseHandle( mapping );
		mapping = NULL;
	}
#endif

	return File::close();
}

void MemoryMappedFile::resize( size_t new_size )
{
	if ( new_size == 0 )
		return;

	if ( new_size < 4 )
		new_size = 4;

#ifdef _WIN32
	if ( start )
		UnmapViewOfFile( start );

	if ( mapping )
		CloseHandle( mapping );

	unsigned int size_low, size_high;
	size_low = lower32( new_size );
	size_high = upper32( new_size );

	if ( new_size != size )
	{
		unsigned int error = SetFilePointer( fd, size_low, ( PLONG ) & size_high, FILE_BEGIN );

		if ( error == INVALID_SET_FILE_POINTER )
		{
			CloseHandle( fd );
			throw PlatformException();
		}

		if ( !SetEndOfFile( fd ) )
		{
			CloseHandle( fd );
			throw PlatformException();
		}
	}

	unsigned long map_flags = PAGE_READONLY;
	if ( ( getFlags() & O_RDWR ) || ( getFlags()  & O_WRONLY ) )
		map_flags = PAGE_READWRITE;

	mapping = CreateFileMapping( fd, NULL, map_flags, size_high, size_low, NULL );

	if ( mapping == NULL )
		throw PlatformException();

	map_flags = FILE_MAP_READ;
	if( ( getFlags() & O_RDWR ) || ( getFlags() & O_WRONLY ) )
		map_flags = FILE_MAP_ALL_ACCESS;

	start = ( char * )MapViewOfFile( mapping, map_flags, 0, 0, 0 );

	if ( start == NULL )
		throw PlatformException();

	size = new_size;
#else
	if ( start )
	{
		writeBack();

		if ( munmap( start, size ) == -1 )
		{
			DiskOperations::close( fd );
			throw PlatformException();
		}
	}

	// enlarge map file

	if ( new_size != size )
	{
		int ret = ftruncate( fd, new_size );

		if ( ret != 0 )
		{
			DiskOperations::close( fd );
			throw PlatformException();
		}

	}

	unsigned long mmap_flags = PROT_READ;
	if( ( getFlags() &  O_RDWR ) == O_RDWR || ( getFlags() == O_WRONLY ) == O_WRONLY )
		mmap_flags |= PROT_WRITE;

	void* mmap_ret = mmap( 0, new_size, mmap_flags, MAP_SHARED, ( int )fd, 0 );

	if ( mmap_ret == MAP_FAILED )
		throw PlatformException();

	start = ( char* )mmap_ret;

	size = new_size;
#endif
}

void MemoryMappedFile::writeBack()
{
#ifdef _WIN32
	FlushViewOfFile( start, 0 );
#else
	msync( start, size, MS_SYNC );
#endif
}

void MemoryMappedFile::writeBack( size_t offset, size_t length )
{
#ifdef _WIN32
	FlushViewOfFile( start + offset, length );
#else
	msync( start + offset, length, MS_SYNC );
#endif
}

void MemoryMappedFile::writeBack( void* ptr, size_t length )
{
#if defined(_WIN32)
	FlushViewOfFile( ptr, length );
#elif defined(__sun)
	msync( ( char* )ptr, length, MS_SYNC );
#else
	msync( ptr, length, MS_SYNC );
#endif
}

