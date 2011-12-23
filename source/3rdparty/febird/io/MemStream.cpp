/* vim: set tabstop=4 : */
#include <febird/io/MemStream.h>
#include <sstream>
#include <stdexcept>
#include <typeinfo>
#include <errno.h>

#if defined(_MSC_VER)
# include <intrin.h>
#pragma intrinsic(_BitScanReverse)
//#pragma intrinsic(_BitScanReverse64)
#endif

#include <boost/version.hpp>
#if BOOST_VERSION < 103301
# include <boost/limits.hpp>
# include <boost/detail/limits.hpp>
#else
# include <boost/detail/endian.hpp>
#endif

namespace febird {

//void MemIO_Base::skip(ptrdiff_t diff)

void throw_EndOfFile(const char* func, size_t want, size_t available)
{
    std::ostringstream oss;
    oss << "in " << func << ", want=" << want
        << ", available=" << available
//		<< ", tell=" << tell() << ", size=" << size()
        ;
    throw EndOfFileException(oss.str().c_str());
}

void throw_OutOfSpace(const char* func, size_t want, size_t available)
{
    std::ostringstream oss;
    oss << "in " << func << ", want=" << want
        << ", available=" << available
//		<< ", tell=" << tell() << ", size=" << size()
        ;
    throw OutOfSpaceException(oss.str().c_str());
}

void MemIO::throw_EndOfFile(const char* func, size_t want)
{
    febird::throw_EndOfFile(func, want, remain());
}

void MemIO::throw_OutOfSpace(const char* func, size_t want)
{
    febird::throw_OutOfSpace(func, want, remain());
}

//////////////////////////////////////////////////////////////////////////

void SeekableMemIO::seek(ptrdiff_t newPos)
{
    assert(newPos >= 0);
    if (newPos < 0 || newPos > m_end - m_beg) {
        std::ostringstream oss;
        oss << "in " << BOOST_CURRENT_FUNCTION
            << "[newPos=" << newPos << ", size=" << m_end << "]";
//		errno = EINVAL;
        throw std::invalid_argument(oss.str());
    }
    m_pos = m_beg + newPos;
}

void SeekableMemIO::seek(ptrdiff_t offset, int origin)
{
    size_t pos;
    switch (origin)
    {
        default:
        {
            std::ostringstream oss;
            oss << "in " << BOOST_CURRENT_FUNCTION
                << "[offset=" << offset << ", origin=" << origin << "(invalid)]";
        //	errno = EINVAL;
            throw std::invalid_argument(oss.str().c_str());
        }
        case 0: pos = (size_t)(0 + offset); break;
        case 1: pos = (size_t)(tell() + offset); break;
        case 2: pos = (size_t)(size() + offset); break;
    }
    seek(pos);
}

// rarely used methods....
//

MemIO SeekableMemIO::range(size_t ibeg, size_t iend) const
{
    assert(ibeg <= iend);
    assert(ibeg <= size());
    assert(iend <= size());
    if (ibeg <= iend && ibeg <= size() && iend <= size())
    {
        return MemIO(m_beg + ibeg, m_beg + iend);
    }
    std::ostringstream oss;
    oss << BOOST_CURRENT_FUNCTION
        << ": size=" << size()
        << ", tell=" << tell()
        << ", ibeg=" << ibeg
        << ", iend=" << iend
        ;
    throw std::invalid_argument(oss.str());
}

//////////////////////////////////////////////////////////////////////////

AutoGrownMemIO::AutoGrownMemIO(size_t size)
{
    if (size) {
        m_beg = (byte*)malloc(size);
        if (NULL == m_beg) {
#ifdef _MSC_VER
            char szMsg[128];
            sprintf(szMsg
                , "AutoGrownMemIO::AutoGrownMemIO(size=%lu)"
                , (unsigned long)size
                );
            throw std::bad_alloc(szMsg);
#else
            throw std::bad_alloc();
#endif
        }
        m_end = m_beg + size;
        m_pos = m_beg;
    }
    else
        m_pos = m_end = m_beg = NULL;
}

AutoGrownMemIO::~AutoGrownMemIO()
{
    if (m_beg)
        free(m_beg);
}

void AutoGrownMemIO::clone(const AutoGrownMemIO& src)
{
    AutoGrownMemIO t(src.size());
    memcpy(t.begin(), src.begin(), src.size());
    this->swap(t);
}

/**
 @brief 改变 buffer 尺寸

  不改变 buffer 中的已存内容，不改变 pos

 @note must m_pos <= newsize
 */
void AutoGrownMemIO::resize(size_t newsize)
{
    assert(tell() <= newsize);

    size_t oldsize = size();
    byte* newbeg = (byte*)realloc(m_beg, newsize);
    if (newbeg) {
        m_pos = newbeg + (m_pos - m_beg);
        m_end = newbeg + newsize;
        m_beg = newbeg;
    }
    else {
#ifdef _MSC_VER
        std::ostringstream oss;
        oss << "realloc failed in \"void AutoGrownMemIO::resize(size[new=" << newsize << ", old=" << oldsize
            << "])\", the AutoGrownMemIO object is not mutated!";
        throw std::bad_alloc(oss.str().c_str());
#else
        throw std::bad_alloc();
#endif
    }
}

/**
 @brief 释放原先的空间并重新分配

  相当于按新尺寸重新构造一个新 AutoGrownMemIO
  不需要把旧内容拷贝到新地址
 */
void AutoGrownMemIO::init(size_t newsize)
{
    size_t oldsize = (size_t)(m_beg - m_beg);
    if (m_beg)
        ::free(m_beg);
    if (newsize) {
        m_beg = (byte*)::malloc(newsize);
        if (NULL == m_beg) {
            m_pos = m_end = NULL;
    #ifdef _MSC_VER
            char szMsg[128];
            sprintf(szMsg
                , "malloc failed in AutoGrownMemIO::init(newsize=%lu), oldsize=%lu"
                , (unsigned long)newsize
                , (unsigned long)oldsize
                );
            throw std::bad_alloc(szMsg);
    #else
            throw std::bad_alloc();
    #endif
        }
        m_pos = m_beg;
        m_end = m_beg + newsize;
    }
    else
        m_pos = m_end = m_beg = NULL;
}

void AutoGrownMemIO::growAndWrite(const void* data, size_t length)
{
    using namespace std;
    size_t nSize = size();
    size_t nGrow = max(length, nSize);
    resize(max(nSize + nGrow, (size_t)64u));
    memcpy(m_pos, data, length);
    m_pos += length;
}

void AutoGrownMemIO::growAndWriteByte(byte b)
{
    using namespace std;
    resize(max(2u * size(), (size_t)64u));
    *m_pos++ = b;
}

size_t AutoGrownMemIO::printf(const char* format, ...)
{
    va_list ap;
    size_t n;
    va_start(ap, format);
    n = this->vprintf(format, ap);
    va_end(ap);
    return n;
}

size_t AutoGrownMemIO::vprintf(const char* format, va_list ap)
{
    while (1) {
        ptrdiff_t n, size = m_end - m_pos;

        /* Try to print in the allocated space. */
        n = ::vsnprintf((char*)m_pos, size, format, ap);

        /* If that worked, return the written bytes. */
        if (n > -1 && n < size) {
            m_pos += n;
            return n;
        }
        /* Else try again with more space. */
        if (n > -1)    /* glibc 2.1 */
            size = n+1; /* precisely what is needed */
        else           /* glibc 2.0 */
            size *= 2;  /* twice the old size */

        this->resize((m_pos - m_beg + size) * 2);
    }
}

///////////////////////////////////////////////////////
//
#ifdef _GNU_SOURCE

ssize_t
MemIO_FILE_read(void *cookie, char *buf, size_t size)
{
    MemIO* input = (MemIO*)cookie;
    return input->read(buf, size);
}

ssize_t
AutoGrownMemIO_FILE_write(void *cookie, const char *buf, size_t size)
{
    AutoGrownMemIO* output = (AutoGrownMemIO*)cookie;
    return output->write(buf, size);
}

int AutoGrownMemIO_FILE_seek(void* cookie, off_t* offset, int whence)
{
    AutoGrownMemIO* output = (AutoGrownMemIO*)cookie;
    try {
        output->seek(*offset, whence);
        *offset = output->tell();
        return 0;
    }
    catch (const std::exception& e) {
        errno = EINVAL;
        return -1;
    }
}

/**
 * @note must call fclose after use of returned FILE
 */
FILE* MemIO::forInputFILE()
{
    cookie_io_functions_t func = {
        MemIO_FILE_read,
        NULL,
        NULL,
        NULL
    };
    assert(this);
    void* cookie = this;
    FILE* fp = fopencookie(cookie,"r", func);
    if (fp == NULL) {
        perror("fopencookie@MemIO::getInputFILE");
        return NULL;
    }
    return fp;
}

/**
 * @note must call fclose after use of returned FILE
 */
FILE* AutoGrownMemIO::forFILE(const char* mode)
{
    cookie_io_functions_t func = {
        MemIO_FILE_read,
        AutoGrownMemIO_FILE_write,
        AutoGrownMemIO_FILE_seek,
        NULL
    };
    assert(this);
    void* cookie = this;
    FILE* fp = fopencookie(cookie, mode, func);
    if (fp == NULL) {
        perror("fopencookie@AutoGrownMemIO::forOutputFILE");
        return NULL;
    }
    return fp;
}

#endif // _GNU_SOURCE

#define STREAM_READER MinMemIO
#define STREAM_WRITER MinMemIO
#include <febird/io/var_int_io.hpp>

#define STREAM_READER MemIO
#define STREAM_WRITER MemIO
#include <febird/io/var_int_io.hpp>

#define STREAM_WRITER AutoGrownMemIO
#include <febird/io/var_int_io.hpp>

} // namespace febird
