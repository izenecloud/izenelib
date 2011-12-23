/* vim: set tabstop=4 : */
#ifndef __febird_io_FileStream_h__
#define __febird_io_FileStream_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <stdio.h>
#include <assert.h>

#include "../stdtypes.h"
#include "../refcount.h"
#include "IOException.h"
#include "IStream.h"

namespace febird {

/**
 @brief FileStream encapsulate FILE* as RefCounter, IInputStream, IOutputStream, ISeekable
 @note
  -# FileStream::eof maybe not thread-safe
  -# FileStream::getByte maybe not thread-safe
  -# FileStream::readByte maybe not thread-safe
  -# FileStream::writeByte maybe not thread-safe
 */
class FEBIRD_DLL_EXPORT FileStream
    : public RefCounter
    , public IInputStream
    , public IOutputStream
    , public ISeekable
{
    DECLARE_NONE_COPYABLE_CLASS(FileStream)
protected:
    FILE* m_fp;

public:
    typedef boost::mpl::true_ is_seekable;

    static bool copyFile(const char* srcPath, const char* dstPath);
    static void ThrowOpenFileException(const char* fpath, const char* mode);

public:
    FileStream(const char* fpath, const char* mode);
    FileStream(int fd, const char* mode);
//	explicit FileStream(FILE* fp = 0) throw() : m_fp(fp) {}
    FileStream() throw() : m_fp(0) {} // 不是我打开的文件，请显式 attach/detach
    ~FileStream() throw() { if (m_fp) ::fclose(m_fp); }

    bool isOpen() const throw() { return 0 != m_fp; }
    operator FILE*() const throw() { return m_fp; }

    void open(const char* fpath, const char* mode);

    //! no throw
    bool xopen(const char* fpath, const char* mode) throw();

    void dopen(int fd, const char* mode) throw();

    void close() throw();

    void attach(::FILE* fp) throw();
    FILE* detach() throw();

    FILE* fp() const throw() { return m_fp; }
#ifdef __USE_MISC
    bool eof() const throw() { return !!feof_unlocked(m_fp); }
    int  getByte() throw() { return fgetc_unlocked(m_fp); }
#else
    bool eof() const throw() { return !!feof(m_fp); }
    int  getByte() throw() { return fgetc(m_fp); }
#endif
    byte readByte() throw(EndOfFileException);
    void writeByte(byte b);

    void ensureRead(void* vbuf, size_t length);
    void ensureWrite(const void* vbuf, size_t length);

    size_t read(void* buf, size_t size) throw();
    size_t write(const void* buf, size_t size) throw();
    void flush();

    void rewind();
    void seek(stream_offset_t offset, int origin);
    void seek(stream_position_t pos);
    stream_position_t tell() const;
    stream_position_t size() const;

    size_t pread(stream_position_t pos, void* vbuf, size_t length);
    size_t pwrite(stream_position_t pos, const void* vbuf, size_t length);

    void disbuf() throw();
};

inline byte FileStream::readByte() throw(EndOfFileException)
{
    assert(m_fp);
#ifdef __USE_MISC
    int ch = fgetc_unlocked(m_fp);
#else
    int ch = fgetc(m_fp);
#endif
    if (-1 == ch)
        throw EndOfFileException(BOOST_CURRENT_FUNCTION);
    return ch;
}

inline void FileStream::writeByte(byte b)
{
    assert(m_fp);
#ifdef __USE_MISC
    if (EOF == fputc_unlocked(b, m_fp))
#else
    if (EOF == fputc(b, m_fp))
#endif
        throw OutOfSpaceException(BOOST_CURRENT_FUNCTION);
}

} // namespace febird

#endif
