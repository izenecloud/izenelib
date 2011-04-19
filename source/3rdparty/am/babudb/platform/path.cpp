// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#include "platform/path.h"
using namespace YIELD;

#ifdef _WIN32
#define UNICODE
#include "platform/windows.h"
#else
#include "platform/platform_types.h" // For MAX_PATH
#include "platform/debug.h"
#include <stdlib.h> // For realpath
#include "iconv.h"
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__sun)
#define ICONV_SOURCE_CAST const char**
#else
#define ICONV_SOURCE_CAST char**
#endif
#endif

#include <utility>
using namespace std;


Path::Path( const Path &other )
: host_charset_path( other.host_charset_path ), utf8_path( other.utf8_path )
#ifdef _WIN32
, wide_path( other.wide_path )
#endif
{ }

Path Path::join( const Path& other ) const
{
#ifdef _WIN32
  if ( wide_path.empty() )
    return other;
  else if ( other.wide_path.empty() )
    return *this;
  else
  {
    wstring combined_wide_path( wide_path );
    if ( combined_wide_path[combined_wide_path.size()-1] != DISK_PATH_SEPARATOR &&
       other.wide_path[0] != DISK_PATH_SEPARATOR )
      combined_wide_path.append( DISK_PATH_SEPARATOR_WIDE_STRING, 1 );
    combined_wide_path.append( other.wide_path );
    return Path( combined_wide_path );
  }
#else
  if ( !utf8_path.empty() && !other.utf8_path.empty() )
  {
    string combined_utf8_path( utf8_path );
    if ( combined_utf8_path[combined_utf8_path.size()-1] != DISK_PATH_SEPARATOR &&
       other.utf8_path[0] != DISK_PATH_SEPARATOR )
      combined_utf8_path.append( DISK_PATH_SEPARATOR_STRING, 1 );
    combined_utf8_path.append( other.utf8_path );
    return Path( combined_utf8_path, false );
  }
  else
  {
    string combined_host_charset_path( host_charset_path );
    if ( combined_host_charset_path[combined_host_charset_path.size()-1] != DISK_PATH_SEPARATOR &&
       other.host_charset_path[0] != DISK_PATH_SEPARATOR )
      combined_host_charset_path.append( DISK_PATH_SEPARATOR_STRING, 1 );
    combined_host_charset_path.append( other.host_charset_path );
    return Path( combined_host_charset_path );
  }
#endif
}

bool Path::operator==( const Path& other ) const
{
#ifdef _WIN32
  return wide_path == other.wide_path;
#else
  return host_charset_path == other.host_charset_path;
#endif
}

bool Path::operator!=( const Path& other ) const
{
#ifdef _WIN32
  return wide_path != other.wide_path;
#else
  return host_charset_path != other.host_charset_path;
#endif
}

void Path::init( const char* path, size_t path_len, bool path_is_host_charset )
{
#ifdef _WIN32
  wchar_t _wide_path[MAX_PATH];
#endif

  if ( path_is_host_charset )
  {
    host_charset_path.assign( path, path_len );
#ifdef _WIN32
    wide_path.assign( _wide_path, MultiByteToWideChar( GetACP(), 0, host_charset_path.c_str(), ( int )host_charset_path.size(), _wide_path, MAX_PATH ) );
#endif
  }
  else
  {
    utf8_path.assign( path, path_len );
#ifdef _WIN32
    wide_path.assign( _wide_path, MultiByteToWideChar( CP_UTF8, 0, utf8_path.c_str(), ( int )utf8_path.size(), _wide_path, MAX_PATH ) );
#else
    MultiByteToMultiByte( "UTF-8", utf8_path, HOST_CHARSET, host_charset_path );
#endif
  }
}

#ifdef _WIN32
Path::Path( const wchar_t* wide_path, size_t wide_path_len )
  : wide_path( wide_path, wide_path_len ? wide_path_len : wcslen( wide_path ) )
{
  char _host_charset_path[MAX_PATH];
  int _host_charset_path_len = WideCharToMultiByte( GetACP(), 0, getWidePath().c_str(), ( int )getWidePath().size(), _host_charset_path, MAX_PATH, 0, 0 );
  host_charset_path.assign( _host_charset_path, _host_charset_path_len );
}

Path::Path( const wstring& wide_path ) : wide_path( wide_path )
{
  char _host_charset_path[MAX_PATH];
  int _host_charset_path_len = WideCharToMultiByte( GetACP(), 0, getWidePath().c_str(), ( int )getWidePath().size(), _host_charset_path, MAX_PATH, 0, 0 );
  host_charset_path.assign( _host_charset_path, _host_charset_path_len );
}
#endif

const string& Path::getUTF8Path()
{
  if ( utf8_path.size() == 0 )
  {
#ifdef _WIN32
    char _utf8_path[MAX_PATH];
    int _utf8_path_len = WideCharToMultiByte( CP_UTF8, 0, getWidePath().c_str(), ( int )getWidePath().size(), _utf8_path, MAX_PATH, 0, 0 );
    utf8_path.assign( _utf8_path, _utf8_path_len );
#else
    if ( host_charset_path.size() == 0 ) DebugBreak();
    MultiByteToMultiByte( HOST_CHARSET, host_charset_path, "UTF-8", utf8_path );
#endif
  }

  return utf8_path;
}

#ifndef _WIN32
void Path::MultiByteToMultiByte( const char* fromcode, const string& frompath, const char* tocode, string& topath )
{
  iconv_t converter;

  if ( ( converter = iconv_open( fromcode, tocode ) ) != ( iconv_t )-1 )
  {
    char* _frompath = ( char* )frompath.c_str(); char _topath[MAX_PATH];
    size_t _frompath_size = utf8_path.size(), _topath_size = MAX_PATH;

    if ( ::iconv( converter, NULL, NULL, ( char** )&_topath, &_topath_size ) != (size_t)-1 &&
         ::iconv( converter, ( ICONV_SOURCE_CAST )&_frompath, &_frompath_size, ( char** )&_topath, &_topath_size ) != (size_t)-1 )
    {
      topath.assign( _topath, _topath_size );
    }
    else
    {
//			cerr << "Path: iconv could not convert path " << frompath << " from code " << fromcode << " to code " << tocode;
      topath = frompath;
    }

    iconv_close( converter );
  }
  else
    DebugBreak();
}
#endif


pair<Path, Path> Path::split() const
{
  string::size_type last_slash = getHostCharsetPath().find_last_of( DISK_PATH_SEPARATOR );

  if ( last_slash == getHostCharsetPath().length() )
    return std::make_pair( *this, Path() );
  else if ( (int)last_slash != -1 )
    return std::make_pair( getHostCharsetPath().substr( 0, last_slash ), getHostCharsetPath().substr( last_slash + 1 ) );
  else
    return std::make_pair( Path(), *this );
}

Path Path::abspath() const
{
#ifdef _WIN32
  wchar_t abspath_buffer[MAX_PATH];
  DWORD abspath_buffer_len = GetFullPathNameW( wide_path.c_str(), MAX_PATH, abspath_buffer, NULL );
  return Path( abspath_buffer, abspath_buffer_len );
#else
  char abspath_buffer[MAX_PATH];
  realpath( host_charset_path.c_str(), abspath_buffer );
  return Path( abspath_buffer );
#endif
}
