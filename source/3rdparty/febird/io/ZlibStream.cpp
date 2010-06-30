/* vim: set tabstop=4 : */
#include <febird/io/ZlibStream.h>

#include <assert.h>
#include <string.h>
#include <sstream>
#include <stdio.h>

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
#	include <io.h>
#   pragma comment(lib, "zlib.lib")
#else
#	include <unistd.h>
#	include <sys/stat.h>
#	include <sys/types.h>
#	include <fcntl.h>
#	include <errno.h>
#endif

#include <zlib.h>

#include <febird/io/byte_io_impl.h>

namespace febird {

void ZlibStreamBase::ThrowOpenFileException(const char* fpath, const char* mode)
{
	std::ostringstream oss;
	oss << "mode=" << mode;
	throw OpenFileException(fpath, oss.str().c_str());
}

// only can call on unopened ZlibInputStream
void ZlibStreamBase::open(const char* fpath, const char* mode)
{
	assert(0 == m_fp);
	m_fp = gzopen(fpath, mode);
	if (0 == m_fp)
		ThrowOpenFileException(fpath, mode);
}

bool ZlibStreamBase::xopen(const char* fpath, const char* mode)
{
	assert(0 == m_fp);
	m_fp = gzopen(fpath, mode);
	return 0 != m_fp;
}

void ZlibStreamBase::dopen(int fd, const char* mode)
{
	assert(0 == m_fp);
	m_fp = gzdopen(fd, mode);
	if (0 == m_fp)
	{
		char szbuf[64];
		sprintf(szbuf, "fd=%d", fd);
		ThrowOpenFileException(szbuf, mode);
	}
}

void ZlibStreamBase::close()
{
	assert(m_fp);
	gzclose((gzFile)m_fp);
	m_fp = 0;
}

ZlibStreamBase::~ZlibStreamBase()
{
	if (m_fp)
		gzclose((gzFile)m_fp);
}

///////////////////////////////////////////////////////////////////////////////////////

ZlibInputStream::ZlibInputStream(const char* fpath, const char* mode)
{
	m_fp = 0;
   	open(fpath, mode);
}

ZlibInputStream::ZlibInputStream(int fd, const char* mode)
{
	m_fp = 0;
   	dopen(fd, mode);
}

bool ZlibInputStream::eof() const
{
	return !!gzeof((gzFile)m_fp);
}

size_t ZlibInputStream::read(void* buf, size_t size) throw()
{
	assert(m_fp);
	return gzread((gzFile)m_fp, buf, size);
}

FEBIRD_GEN_ensureRead (ZlibInputStream::)

///////////////////////////////////////////////////////

ZlibOutputStream::ZlibOutputStream(const char* fpath, const char* mode)
{
	m_fp = 0;
   	open(fpath, mode);
}

ZlibOutputStream::ZlibOutputStream(int fd, const char* mode)
{
	m_fp = 0;
   	dopen(fd, mode);
}

void ZlibOutputStream::flush()
{
	assert(m_fp);
	if (gzflush((gzFile)m_fp, Z_SYNC_FLUSH) == EOF)
		throw DelayedWriteFailException(BOOST_CURRENT_FUNCTION);
}

size_t ZlibOutputStream::write(const void* buf, size_t size) throw()
{
	assert(m_fp);
	return gzwrite((gzFile)m_fp, buf, size);
}
FEBIRD_GEN_ensureWrite(ZlibOutputStream::)

} // namespace febird

