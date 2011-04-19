// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#include "platform/stat.h"
#include "platform/disk_operations.h"
#include "platform/platform_exception.h"
using namespace YIELD;

#ifdef _WIN32

#define UNICODE
#include "windows.h"

inline uint64_t WIN2UNIX( uint64_t time ) { 
  return ( ( time - ( uint64_t )116444736000000000LL ) / ( uint64_t )10000000LL );
}

inline int64_t getUnixUTCTimeFromFILETIME( FILETIME& file_time ){
  return WIN2UNIX( create_uint64( file_time.dwHighDateTime, file_time.dwLowDateTime ) );
}

#else

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h> // For O_*
#include <unistd.h>

#endif


void Stat::init( const Path& path )
{
#ifdef _WIN32
  fd_t fd = CreateFileW( path.getWidePath().c_str(), GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL ); // FILE_FLAG_BACKUP_SEMANTICS is necessary to open directories
#else
  fd_t fd = ::open( path.getHostCharsetPath().c_str(), O_RDONLY, S_IRWXU );
#endif

  if ( fd != INVALID_HANDLE_VALUE )
  {
    try { init( fd ); DiskOperations::close( fd ); }
    catch ( std::exception& ) { DiskOperations::close( fd ); throw; }
  }
  else
    throw PlatformException();
}

void Stat::init( fd_t fd )
{
#ifdef _WIN32
  BY_HANDLE_FILE_INFORMATION temp_file_info;
  if ( GetFileInformationByHandle( fd, &temp_file_info ) != 0 )
  {
    type = ( temp_file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ? Directory : File;
    _st_size = ( size_t )create_uint64( temp_file_info.nFileSizeHigh, temp_file_info.nFileSizeLow );
    _st_ctime = getUnixUTCTimeFromFILETIME( temp_file_info.ftCreationTime );
    _st_atime = getUnixUTCTimeFromFILETIME( temp_file_info.ftLastAccessTime );
    _st_mtime = getUnixUTCTimeFromFILETIME( temp_file_info.ftLastWriteTime );
    is_hidden = ( temp_file_info.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN ) != 0;
  }
  else
    throw PlatformException();
#else
  struct stat temp_stat;
  if ( fstat( fd, &temp_stat ) != -1 )
  {
    type = S_ISDIR( temp_stat.st_mode ) ? Directory : File;
    _st_size = temp_stat.st_size;
    _st_ctime = ( int64_t )temp_stat.st_ctime;
    _st_atime = ( int64_t )temp_stat.st_atime;
    _st_mtime = ( int64_t )temp_stat.st_mtime;
    is_hidden = false;

  }
  else
    throw PlatformException();
#endif

}

Stat::Stat( StatType type, size_t _st_size, int64_t _st_mtime, int64_t _st_ctime, int64_t _st_atime, bool is_hidden )
: type( type ), _st_size( _st_size ), _st_mtime( _st_mtime ), _st_ctime( _st_ctime ), _st_atime( _st_atime ), is_hidden( is_hidden )
{ }

Stat::Stat( const Stat& other )
: type( other.type ), _st_size( other._st_size ), _st_mtime( other._st_mtime ), _st_ctime( other._st_ctime ), _st_atime( other._st_atime ), is_hidden( other.is_hidden )
{ }

