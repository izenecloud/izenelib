// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#include "platform/file.h"
#include "platform/path.h"
#include "platform/stat.h"
using namespace YIELD;

#ifdef _WIN32
#include "platform/windows.h"
#undef CopyFile
#else
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600
#endif
#define _LARGEFILE64_SOURCE 1
#define _FILE_OFFSET_BITS 64
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef YIELD_HAVE_POSIX_FILE_AIO
#include <aio.h>
#endif
#if defined(__linux__)
// Mac OS X's off_t is already 64-bit
#define lseek lseek64
#elif defined(__sun)
extern off64_t lseek64(int, off64_t, int);
#define lseek lseek64
#endif
#endif


#ifdef YIELD_HAVE_DISK_AIO
typedef struct
{
#if defined(_WIN32)
  OVERLAPPED overlapped;
#elif defined(YIELD_HAVE_POSIX_FILE_AIO)
  struct aiocb aiocb;
#else
#error
#endif
  File::aio_read_completion_routine_t aio_read_completion_routine;
  void* aio_read_completion_routine_context;
} aio_read_operation_t;

typedef struct
{
#if defined(_WIN32)
  OVERLAPPED overlapped;
#elif defined(YIELD_HAVE_POSIX_FILE_AIO)
  struct aiocb aiocb;
#else
#error
#endif
  File::aio_write_completion_routine_t aio_write_completion_routine;
  void* aio_write_completion_routine_context;
} aio_write_operation_t;
#endif


File::File( const Path& path, unsigned long flags )
: flags( flags )
{
  fd = DiskOperations::open( path, flags );
}

#ifdef _WIN32
File::File( char* path, unsigned long flags )
: flags( flags )
{
  fd = DiskOperations::open( path, flags );
}

File::File( const char* path, unsigned long flags )
: flags( flags )
{
  fd = DiskOperations::open( path, flags );
}
#endif

File* File::open( const Path& path, unsigned long flags )
{
  fd_t fd = DiskOperations::open( path, flags );
  if ( fd != INVALID_HANDLE_VALUE )
    return new File( fd );
  else
    return NULL;
}

File::File( fd_t fd, unsigned long flags ) : fd( fd ), flags( flags )
{ }

File::File( const File& other ) : fd( other.fd ), flags( other.flags )
{ }

File::File() : fd( INVALID_HANDLE_VALUE ), flags( 0 )
{ }

File::~File()
{
  if ( ( flags & O_CLOSE_ON_DESTRUCT ) == O_CLOSE_ON_DESTRUCT )
    close();
}

uint64_t File::seek( uint64_t offset, unsigned char whence )
{
#ifdef _WIN32
  LONG dwDistanceToMove = lower32( offset ), dwDistanceToMoveHigh = upper32( offset );
  return SetFilePointer( fd, dwDistanceToMove, &dwDistanceToMoveHigh, whence );
#else
  return lseek( fd, ( off_t )offset, whence );
#endif
}

ssize_t File::read( void* buf, size_t nbyte )
{
#ifdef _WIN32
  DWORD dwBytesRead;
  if ( ReadFile( fd, buf, nbyte, &dwBytesRead, NULL ) )
    return dwBytesRead;
  else
    return -1;
#else
  return ::read( fd, buf, nbyte );
#endif
}

ssize_t File::write( const void* buf, size_t nbyte )
{
#ifdef _WIN32
  DWORD dwBytesWritten;
  if ( WriteFile( fd, buf, nbyte, &dwBytesWritten, NULL ) )
    return dwBytesWritten;
  else
    return -1;
#else
  return ::write( fd, buf, nbyte );
#endif
}

#ifdef YIELD_HAVE_DISK_AIO
int File::aio_read( void* buf, size_t nbyte, aio_read_completion_routine_t aio_read_completion_routine, void* aio_read_completion_routine_context )
{
  aio_read_operation_t* aio_read_op = new aio_read_operation_t;
  memset( aio_read_op, 0, sizeof( *aio_read_op ) );
  aio_read_op->aio_read_completion_routine = aio_read_completion_routine;
  aio_read_op->aio_read_completion_routine_context = aio_read_completion_routine_context;

#if defined(_WIN32)
  if ( ( getFlags() & O_ASYNC ) != O_ASYNC )
  {
    SetLastError( ERROR_INVALID_HANDLE );
    return -1;
  }

  if ( ReadFileEx( fd, buf, ( DWORD )nbyte, ( LPOVERLAPPED )aio_read_op, ( LPOVERLAPPED_COMPLETION_ROUTINE )overlapped_read_completion ) != 0 )
    return 0;
  else
    return -1;
#elif defined(YIELD_HAVE_POSIX_FILE_AIO)
  aio_read_op->aiocb.aio_fildes = fd;
  aio_read_op->aiocb.aio_buf = buf;
  aio_read_op->aiocb.aio_nbytes = nbyte;
  aio_read_op->aiocb.aio_offset = 0;
  aio_read_op->aiocb.aio_sigevent.sigev_notify = SIGEV_THREAD;
  aio_read_op->aiocb.aio_sigevent.sigev_notify_function = aio_read_notify;
  aio_read_op->aiocb.aio_sigevent.sigev_value.sival_ptr = aio_read_op;
  return ::aio_read( &aio_read_op->aiocb );
#endif
}

int File::aio_write( const void* buf, size_t nbyte, aio_write_completion_routine_t aio_write_completion_routine, void* aio_write_completion_routine_context )
{
  aio_write_operation_t* aio_write_op = new aio_write_operation_t;
  memset( aio_write_op, 0, sizeof( *aio_write_op ) );
  aio_write_op->aio_write_completion_routine = aio_write_completion_routine;
  aio_write_op->aio_write_completion_routine_context = aio_write_completion_routine_context;

#if defined(_WIN32)
  if ( ( getFlags() & O_ASYNC ) != O_ASYNC )
  {
    SetLastError( ERROR_INVALID_HANDLE );
    return -1;
  }

  if ( WriteFileEx( fd, buf, ( DWORD )nbyte, ( LPOVERLAPPED )aio_write_op, ( LPOVERLAPPED_COMPLETION_ROUTINE )overlapped_write_completion ) != 0 )
    return 0;
  else
    return -1;
#elif defined(YIELD_HAVE_POSIX_FILE_AIO)
  aio_write_op->aiocb.aio_fildes = fd;
  aio_write_op->aiocb.aio_buf = ( void* )buf;
  aio_write_op->aiocb.aio_nbytes = nbyte;
  aio_write_op->aiocb.aio_offset = 0;
  aio_write_op->aiocb.aio_sigevent.sigev_notify = SIGEV_THREAD;
  aio_write_op->aiocb.aio_sigevent.sigev_notify_function = aio_write_notify;
  aio_write_op->aiocb.aio_sigevent.sigev_value.sival_ptr = aio_write_op;
  return ::aio_write( &aio_write_op->aiocb );
#endif
}

#if defined(_WIN32)

void __stdcall File::overlapped_read_completion( DWORD dwError, DWORD dwBytesTransferred, LPVOID lpOverlapped )
{
  aio_read_operation_t* aio_read_op = ( aio_read_operation_t* )lpOverlapped;
  aio_read_op->aio_read_completion_routine( dwError, dwBytesTransferred, aio_read_op->aio_read_completion_routine_context );
  delete aio_read_op;
}

void __stdcall File::overlapped_write_completion( DWORD dwError, DWORD dwBytesTransferred, LPVOID lpOverlapped )
{
  aio_write_operation_t* aio_write_op = ( aio_write_operation_t* )lpOverlapped;
  aio_write_op->aio_write_completion_routine( dwError, dwBytesTransferred, aio_write_op->aio_write_completion_routine_context );
  delete aio_write_op;
}

#elif defined(YIELD_HAVE_POSIX_FILE_AIO)

void File::aio_read_notify( sigval_t sigval )
{
  aio_read_operation_t* aio_read_op = ( aio_read_operation_t* )sigval.sival_ptr;
  aio_read_op->aio_read_completion_routine( ( unsigned long )aio_error( &aio_read_op->aiocb ), ( size_t )aio_return( &aio_read_op->aiocb ), aio_read_op->aio_read_completion_routine_context );
  delete aio_read_op;
}

void File::aio_write_notify( sigval_t sigval )
{
  aio_write_operation_t* aio_write_op = ( aio_write_operation_t* )sigval.sival_ptr;
  aio_write_op->aio_write_completion_routine( ( unsigned long )aio_error( &aio_write_op->aiocb ), ( size_t )aio_return( &aio_write_op->aiocb ), aio_write_op->aio_write_completion_routine_context );
  delete aio_write_op;
}

#endif

#endif

bool File::close()
{
  if ( fd != INVALID_HANDLE_VALUE )
  {
    bool ret = DiskOperations::close( fd, flags );
    fd = INVALID_HANDLE_VALUE;
    return ret;
  }
  else
    return true;
}


bool File::isOpen() 
{
  return fd != INVALID_HANDLE_VALUE;
}

/*
uint64_t AIOFile::seek( uint64_t offset, unsigned char whence )
{
#ifdef _WIN32
  return ( aioe.aiocb.Offset = ( ULONG )File::seek( offset, whence ) );
#else
  YIELD::DebugBreak();
#endif
}
*/


bool File::CopyFile(const std::string& from, const std::string& to) {
  File* source = File::open(from, O_RDONLY);
  if (source == NULL)
    return false;
  File* dest = File::open(to, O_WRONLY|O_TRUNC|O_CREAT);
  if (dest == NULL)
    return false;

  const ssize_t buffer_size = 1024 * 1024;
  char* buffer = new char[buffer_size];

  ssize_t read = buffer_size;
  while (read == buffer_size) {
    read = source->read(buffer, buffer_size);
    if (dest->write(buffer, read) != read) {
      return false;
    }
  }

  delete buffer;
  delete source;
  delete dest;
  return true;
}
