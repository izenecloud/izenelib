// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#ifndef YIELD_PLATFORM_FILE_H
#define YIELD_PLATFORM_FILE_H

#include "yield/platform/disk_operations.h"

#if defined(_WIN32) || defined(YIELD_HAVE_POSIX_FILE_AIO) // Solaris does not support AIO callbacks (?)
#define YIELD_HAVE_DISK_AIO
#if defined(YIELD_HAVE_POSIX_FILE_AIO)
#include <signal.h> // For sigval_t
#endif
#endif


namespace YIELD
{
  class Path;


  class File
  {
  public:
#ifdef YIELD_HAVE_DISK_AIO
    typedef void ( *aio_read_completion_routine_t )( unsigned long error_code, size_t nbyte, void* context );
    typedef void ( *aio_write_completion_routine_t )( unsigned long error_code, size_t nbyte, void* context );
#endif

    File( const Path& path, unsigned long flags = O_RDONLY|O_THROW_EXCEPTIONS|O_CLOSE_ON_DESTRUCT );
#ifdef _WIN32
    // Since fd_t is void* on Windows we need these to delegate to the const Path& variant rather than the fd_t one
    File( char* path, unsigned long flags = O_RDONLY|O_THROW_EXCEPTIONS|O_CLOSE_ON_DESTRUCT );
    File( const char* path, unsigned long flags = O_RDONLY|O_THROW_EXCEPTIONS|O_CLOSE_ON_DESTRUCT );
#endif
    static File* open( const Path& path, unsigned long flags = O_RDONLY|O_CLOSE_ON_DESTRUCT );
    explicit File( fd_t fd, unsigned long flags = O_THROW_EXCEPTIONS|O_CLOSE_ON_DESTRUCT );
    File( const File& );
    File();
    virtual ~File();

    inline unsigned long getFlags() { return flags; }
    inline fd_t getFD() { return fd; }	

    uint64_t seek( uint64_t offset, unsigned char whence );
    ssize_t read( void* buf, size_t nbyte );
    ssize_t write( const void* buf, size_t nbyte );
    ssize_t write( const std::string& buf ) { return write( buf.c_str(), buf.size() ); }
    ssize_t write( const char* buf ) { return write( buf, std::strlen( buf ) ); }
#ifdef YIELD_HAVE_DISK_AIO
    int aio_read( void* buf, size_t nbyte, aio_read_completion_routine_t, void* context );
    int aio_write( const void* buf, size_t nbyte, aio_write_completion_routine_t, void* context );
#endif
    virtual bool close();
    bool isOpen();

    static bool CopyFile(const std::string& from, const std::string& to);

  protected:
    fd_t fd;
    unsigned long flags;

  private:
#if defined(_WIN32)
    static void __stdcall overlapped_read_completion( unsigned long, unsigned long, void* );
    static void __stdcall overlapped_write_completion( unsigned long, unsigned long, void* );
#elif defined(YIELD_HAVE_POSIX_FILE_AIO)
    static void aio_read_notify( sigval_t );
    static void aio_write_notify( sigval_t );
#endif
  };
};

#endif
