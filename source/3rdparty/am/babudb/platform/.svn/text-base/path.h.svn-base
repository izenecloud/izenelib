// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#ifndef YIELD_PLATFORM_PATH_H
#define YIELD_PLATFORM_PATH_H

#include "yield/platform/platform_config.h" // for YIELD

#include <string>
#include <cstring>
#include <utility>

#ifdef _WIN32
#define DISK_PATH_SEPARATOR '\\'
#define DISK_PATH_SEPARATOR_STRING "\\"
#define DISK_PATH_SEPARATOR_WIDE_STRING L"\\"
#else
#define HOST_CHARSET "UTF-8"
#define DISK_PATH_SEPARATOR '/'
#define DISK_PATH_SEPARATOR_STRING "/"
#endif


namespace YIELD
{
  class Path
  {
  public:
    Path() { }
    Path( const char* path, size_t path_len = 0, bool path_is_host_charset = true ) { init( path, ( path_len != 0 ) ? path_len : std::strlen( path ), path_is_host_charset ); }
    Path( const std::string& path, bool path_is_host_charset = true ) { init( path.c_str(), path.size(), path_is_host_charset ); }
#ifdef _WIN32
    Path( const wchar_t* wide_path, size_t wide_path_len = 0 );
    Path( const std::wstring& wide_path );
#endif
    Path( const Path& );

    // These functions return the platform path, e.g. with \\ on Windows
    const std::string& getHostCharsetPath() const { return host_charset_path; }
    const std::string& getUTF8Path();
#ifdef _WIN32
    const std::wstring& getWidePath() const { return wide_path; }
    operator const std::wstring&() { return wide_path; }
    operator const wchar_t*() { return wide_path.c_str(); }
#endif
    operator const std::string&() { return host_charset_path; }
    Path operator+( const Path& other ) const { return join( other ); }
    bool operator==( const Path& ) const;
    bool operator!=( const Path& ) const;

    Path join( const Path& ) const;
    std::pair<Path, Path> split() const;
    Path abspath() const;

  private:
    void init( const char*, size_t, bool );

    std::string host_charset_path, utf8_path;
#ifdef _WIN32
    std::wstring wide_path;
#else
    void MultiByteToMultiByte( const char* fromcode, const std::string& frompath, const char* tocode, std::string& topath );
#endif
  };
};

#endif
