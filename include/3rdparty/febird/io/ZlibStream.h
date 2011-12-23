/* vim: set tabstop=4 : */
#ifndef __febird_io_ZlibStream_h__
#define __febird_io_ZlibStream_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include "../stdtypes.h"
#include "../refcount.h"
#include "IOException.h"
#include "IStream.h"

namespace febird {

class FEBIRD_DLL_EXPORT ZlibStreamBase : public RefCounter
{
protected:
	void* m_fp;
	void ThrowOpenFileException(const char* fpath, const char* mode);

public:
	ZlibStreamBase() : m_fp(0) {}
	virtual ~ZlibStreamBase();

	bool isOpen() const { return 0 != m_fp; }

	void open(const char* fpath, const char* mode);
	bool xopen(const char* fpath, const char* mode);
	void dopen(int fd, const char* mode);
	void close();
};

class FEBIRD_DLL_EXPORT ZlibInputStream	: public IInputStream, public ZlibStreamBase
{
	DECLARE_NONE_COPYABLE_CLASS(ZlibInputStream)

public:
	explicit ZlibInputStream(const char* fpath, const char* mode = "rb");
	explicit ZlibInputStream(int fd, const char* mode = "rb");
	ZlibInputStream() throw() {} // 不是我打开的文件，请显式 attach/detach

	bool eof() const;

	void ensureRead(void* vbuf, size_t length);
	size_t read(void* buf, size_t size) throw();
};

class FEBIRD_DLL_EXPORT ZlibOutputStream : public IOutputStream, public ZlibStreamBase
{
	DECLARE_NONE_COPYABLE_CLASS(ZlibOutputStream)

public:
	explicit ZlibOutputStream(const char* fpath, const char* mode = "wb");
	explicit ZlibOutputStream(int fd, const char* mode = "wb");
	ZlibOutputStream() throw() {} // 不是我打开的文件，请显式 attach/detach

	void ensureWrite(const void* vbuf, size_t length);
	size_t write(const void* buf, size_t size) throw();
	void flush();
};

} // namespace febird

#endif

