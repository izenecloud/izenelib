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
	bool eof() const throw() { return !!feof(m_fp); }
	int  getByte() throw() { return fgetc(m_fp); }
	byte readByte() throw(EndOfFileException);
	void writeByte(byte b);

	void ensureRead(void* vbuf, size_t length);
	void ensureWrite(const void* vbuf, size_t length);

	size_t read(void* buf, size_t size) throw();
	size_t write(const void* buf, size_t size) throw();
	void flush();

	void seek(stream_offset_t offset, int origin);
	void seek(stream_position_t pos);
	stream_position_t tell();
	stream_position_t size() const throw();

	void disbuf() throw();
};

inline byte FileStream::readByte() throw(EndOfFileException)
{
	assert(m_fp);
	int ch = fgetc(m_fp);
	if (-1 == ch)
		throw EndOfFileException(BOOST_CURRENT_FUNCTION);
	return ch;
}

inline void FileStream::writeByte(byte b)
{
	assert(m_fp);
	if (EOF == fputc(b, m_fp))
		throw OutOfSpaceException(BOOST_CURRENT_FUNCTION);
}

} // namespace febird

#endif

