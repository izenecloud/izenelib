/* vim: set tabstop=4 : */
#ifndef __febird_io_AutoGrownMemIO_h__
#define __febird_io_AutoGrownMemIO_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <assert.h>
#include <string.h> // for memcpy
//#include <stdarg.h>
//#include <stdio.h>  // for size_t
#include <stdexcept>
#include <boost/current_function.hpp>
//#include <boost/type_traits/integral_constant.hpp>
#include <boost/mpl/bool.hpp>

#include "../stdtypes.h"
#include "IOException.h"

namespace febird {

FEBIRD_DLL_EXPORT void throw_EndOfFile(const char* func, size_t want, size_t available);
FEBIRD_DLL_EXPORT void throw_OutOfSpace(const char* func, size_t want, size_t available);

//! MinMemIO
//! +--MemIO
//!    +--SeekableMemIO
//!       +--AutoGrownMemIO

/**
 @brief 最有效的MemIO
 
 只用一个指针保存当前位置，没有范围检查，只应该在完全可预测的情况下使用这个类

 @note
  -# 如果无法预测是否会越界，禁止使用该类
 */
class FEBIRD_DLL_EXPORT MinMemIO
{
public:
	typedef boost::mpl::false_ is_seekable; //!< 不能 seek

	void set(void* FEBIRD_RESTRICT vptr) { m_pos = (unsigned char*)vptr; }

	byte readByte() FEBIRD_RESTRICT { return *m_pos++; }
	int  getByte() FEBIRD_RESTRICT { return *m_pos++; }
	void writeByte(byte b) FEBIRD_RESTRICT { *m_pos++ = b; }

	void ensureRead(void* FEBIRD_RESTRICT data, size_t length) FEBIRD_RESTRICT 
	{
		memcpy(data, m_pos, length);
		m_pos += length;
	}
	void ensureWrite(const void* FEBIRD_RESTRICT data, size_t length) FEBIRD_RESTRICT 
	{
		memcpy(m_pos, data, length);
		m_pos += length;
	}

	size_t read(void* FEBIRD_RESTRICT data, size_t length) FEBIRD_RESTRICT 
	{
		memcpy(data, m_pos, length);
		m_pos += length;
		return length;
	}
	size_t write(const void* FEBIRD_RESTRICT data, size_t length) FEBIRD_RESTRICT 
	{
		memcpy(m_pos, data, length);
		m_pos += length;
		return length;
	}

	void flush() {} // do nothing...

	byte* current() const { return m_pos; }

	//! caller can use this function to determine an offset difference
	ptrdiff_t diff(const void* FEBIRD_RESTRICT start) const throw() { return m_pos - (byte*)start; }

	void skip(ptrdiff_t diff) throw() {	m_pos += diff; }

	byte uncheckedReadByte() FEBIRD_RESTRICT { return *m_pos++; }
	void uncheckedWriteByte(byte b) FEBIRD_RESTRICT { *m_pos++ = b; }

	template<class InputStream>
	void from_input(InputStream& input, size_t length) FEBIRD_RESTRICT 
	{
		input.ensureRead(m_pos, length);
		m_pos += length;
	}
	template<class OutputStream>
	void to_output(OutputStream& output, size_t length) FEBIRD_RESTRICT 
	{
		output.ensureWrite(m_pos, length);
		m_pos += length;
	}

protected:
	byte* FEBIRD_RESTRICT m_pos;
};

/**
 @brief Mem Stream 操作所需的最小集合
  
  这个类的尺寸非常小，在极端情况下效率非常高，在使用外部提供的缓冲时，这个类是最佳的选择
  这个类可以拷贝
 */
class FEBIRD_DLL_EXPORT MemIO : public MinMemIO
{
public:
	MemIO() {}
	MemIO(void* buf, size_t size) { set(buf, size); }
	MemIO(void* beg, void* end) { set(beg, end); }

	void set(void* buf, size_t size)
	{
		m_pos = (byte*)buf;
		m_end = (byte*)buf + size;
	}
	void set(void* beg, void* end)
	{
		m_pos = (byte*)beg;
		m_end = (byte*)end;
	}

	//! test pos reach end or not
	bool eof() const throw()
	{
		assert(m_pos <= m_end);
		return m_pos == m_end;
	}

	byte readByte() FEBIRD_RESTRICT throw(EndOfFileException);
	int  getByte() FEBIRD_RESTRICT throw();
	void writeByte(byte b) FEBIRD_RESTRICT throw(OutOfSpaceException);

	void ensureRead(void* FEBIRD_RESTRICT data, size_t length) FEBIRD_RESTRICT ;
	void ensureWrite(const void* FEBIRD_RESTRICT data, size_t length)FEBIRD_RESTRICT ;

	size_t read(void* FEBIRD_RESTRICT data, size_t length) throw();
	size_t write(const void* FEBIRD_RESTRICT data, size_t length) throw();

	// rarely used methods....
	//
	size_t available() const throw() { return m_end - m_pos; }
	byte*  end() const throw() { return m_end; }

	/**
	 @brief 向前跳过 @a diff 个字节
	 @a 可以是负数，表示向后跳跃
	 */
	void skip(ptrdiff_t diff)
	{
		assert(m_pos + diff <= m_end);
		if (m_pos + diff <= m_end)
			m_pos += diff;
		else
			throw std::invalid_argument("diff is too large");
	}

	template<class InputStream>
	void from_input(InputStream& input, size_t length)FEBIRD_RESTRICT 
	{
		if (m_pos + length > m_end)
			throw_OutOfSpace(BOOST_CURRENT_FUNCTION, length);
		input.ensureRead(m_pos, length);
		m_pos += length;
	}
	template<class OutputStream>
	void to_output(OutputStream& output, size_t length)FEBIRD_RESTRICT 
	{
		if (m_pos + length > m_end)
			throw_EndOfFile(BOOST_CURRENT_FUNCTION, length);
		output.ensureWrite(m_pos, length);
		m_pos += length;
	}

protected:
	void throw_EndOfFile(const char* func, size_t want);
	void throw_OutOfSpace(const char* func, size_t want);

protected:
	byte* FEBIRD_RESTRICT m_end; // only used by set/eof
};

class FEBIRD_DLL_EXPORT AutoGrownMemIO;

class FEBIRD_DLL_EXPORT SeekableMemIO : public MemIO
{
public:
	typedef boost::mpl::true_ is_seekable; //!< 可以 seek

	SeekableMemIO() { m_pos = m_beg = m_end = 0; }
	SeekableMemIO(void* buf, size_t size) { set(buf, size); }
	SeekableMemIO(void* beg, void* end) { set(beg, end); }
	SeekableMemIO(const MemIO& x) { set(x.current(), x.end()); }

	void set(void* buf, size_t size) throw()
	{
		m_pos = (byte*)buf;
		m_beg = (byte*)buf;
		m_end = (byte*)buf + size;
	}
	void set(void* beg, void* end) throw()
	{
		m_pos = (byte*)beg;
		m_beg = (byte*)beg;
		m_end = (byte*)end;
	}

	byte*  begin()const throw() { return m_beg; }
	byte*  buf()  const throw() { return m_beg; }
	size_t size() const throw() { return m_end-m_beg; }

	size_t tell() const throw() { return m_pos-m_beg; }

	void rewind() throw() { m_pos = m_beg; }
	void seek(long newPos);
	void seek(long offset, int origin);

	void swap(SeekableMemIO& that);

	//@{
	//! return part of (*this) as a MemIO
	MemIO range(size_t ibeg, size_t iend) const;
	MemIO head() const throw() { return MemIO(m_beg, m_pos); }
	MemIO tail() const throw() { return MemIO(m_pos, m_end); }
	MemIO whole()const throw() { return MemIO(m_beg, m_end); }
	//@}

protected:
	byte* FEBIRD_RESTRICT m_beg;

private:
	SeekableMemIO(AutoGrownMemIO&);
	SeekableMemIO(const AutoGrownMemIO&);
};

/**
 @brief AutoGrownMemIO 可以管理自己的 buffer

 @note
  - 如果只需要 Eofmark, 使用 MemIO 就可以了
  - 如果还需要 seekable, 使用 SeekableMemIO
 */
//template<bool Use_c_malloc>
class FEBIRD_DLL_EXPORT AutoGrownMemIO : public SeekableMemIO
{
	DECLARE_NONE_COPYABLE_CLASS(AutoGrownMemIO);

	void growAndWrite(const void* FEBIRD_RESTRICT data, size_t length)FEBIRD_RESTRICT ;
	void growAndWriteByte(byte b)FEBIRD_RESTRICT ;

public:
	explicit AutoGrownMemIO(size_t size = 0);

	~AutoGrownMemIO();

	void writeByte(byte b) FEBIRD_RESTRICT 
	{
		assert(m_pos <= m_end);

		if (m_pos < m_end)
			*m_pos++ = b;
		else
			growAndWriteByte(b);
	}

	void ensureWrite(const void* data, size_t length) FEBIRD_RESTRICT 
	{
		assert(m_pos <= m_end);

		if (m_pos + length <= m_end) {
			::memcpy(m_pos, data, length);
			m_pos += length;
		} else
			growAndWrite(data, length);
	}

	size_t write(const void* data, size_t length) FEBIRD_RESTRICT throw()
	{
		ensureWrite(data, length);
		return length;
	}
/*
	void printf(const char* format, ...)
#ifdef __GNUC__
	__THROW __attribute__ ((__format__ (__printf__, 1, 2)))
#endif
	;

	void vprintf(const char* format, av_list ap);
#ifdef __GNUC__
	__THROW __attribute__ ((__format__ (__printf__, 1, 0)))
#endif
	;
*/
	void clone(const AutoGrownMemIO& src);

	// rarely used methods....
	//
	void resize(size_t newsize);
	void init(size_t size);

	template<class InputStream>
	void from_input(InputStream& input, size_t length) FEBIRD_RESTRICT 
	{
		if (m_pos + length > m_end)
			resize(tell() + length);
		input.ensureRead(m_pos, length);
		m_pos += length;
	}

	void swap(AutoGrownMemIO& that);

private:
	//@{
	//! disable MemIO::set
	//!
	void set(void* buf, size_t size);
	void set(void* beg, void* end);
	//@}

	//@{
	//! disable convert-ability to MemIO
	//! this cause gcc warning: conversion to a reference to a base class will never use a type conversion operator
	//! see SeekableMemIO::SeekableMemIO(const AutoGrownMemIO&)
//	operator const SeekableMemIO&() const;
//	operator SeekableMemIO&();
	//@}
};

//////////////////////////////////////////////////////////////////////////

/**
 * @brief 读取 length 长的数据到 data
 * 
 * 这个函数还是值得 inline 的，可以参考如下手工的汇编代码：
 *
 * inlined in caller, 省略了寄存器保存和回复指令，实际情况下也有可能不用保存和恢复
 *   mov eax, m_end
 *   sub eax, m_pos
 *   mov ecx, length
 *   mov esi, m_pos
 *   mov edi, data
 *   cld
 *   cmp eax, ecx
 *   jl  Overflow
 *   rep movsb
 *   jmp End
 * Overflow:
 *   mov ecx, eax
 *   rep movsb
 * End:
 *   mov m_pos, esi
 * --------------------------------
 * sub routine in caller:
 *   push length
 *   push data
 *   push this
 *   call MemIO::read
 *   add  esp, 12 ; 如果是 stdcall, 则没有这条语句
 */
inline size_t MemIO::read(void* FEBIRD_RESTRICT data, size_t length) FEBIRD_RESTRICT throw()
{
	register int n = m_end - m_pos;
	if (n < int(length)) {
		::memcpy(data, m_pos, n);
	//	m_pos = m_end;
		m_pos += n;
		return n;
	} else {
		::memcpy(data, m_pos, length);
		m_pos += length;
		return length;
	}
}

inline size_t MemIO::write(const void* FEBIRD_RESTRICT data, size_t length) FEBIRD_RESTRICT throw()
{
	register int n = m_end - m_pos;
	if (n < int(length)) {
		::memcpy(m_pos, data, n);
	//	m_pos = m_end;
		m_pos += n;
		return n;
	} else {
		::memcpy(m_pos, data, length);
		m_pos += length;
		return length;
	}
}

inline void MemIO::ensureRead(void* FEBIRD_RESTRICT data, size_t length)FEBIRD_RESTRICT 
{
	if (m_pos + length <= m_end) {
		::memcpy(data, m_pos, length);
		m_pos += length;
	} else
		throw_EndOfFile(BOOST_CURRENT_FUNCTION, length);
}

inline void MemIO::ensureWrite(const void* FEBIRD_RESTRICT data, size_t length)FEBIRD_RESTRICT 
{
	if (m_pos + length <= m_end) {
		::memcpy(m_pos, data, length);
		m_pos += length;
	} else
		throw_OutOfSpace(BOOST_CURRENT_FUNCTION, length);
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4715) // not all control paths return a value
#endif

inline byte MemIO::readByte() FEBIRD_RESTRICT throw(EndOfFileException)
{
	if (m_pos < m_end)
		return *m_pos++;
	else
		throw_EndOfFile(BOOST_CURRENT_FUNCTION, 1);return 0;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

inline void MemIO::writeByte(byte b) FEBIRD_RESTRICT throw(OutOfSpaceException)
{
	if (m_pos < m_end)
		*m_pos++ = b;
	else
		throw_OutOfSpace(BOOST_CURRENT_FUNCTION, 1);
}
inline int MemIO::getByte() FEBIRD_RESTRICT throw()
{
	if (m_pos < m_end)
		return *m_pos++;
	else
		return -1;
}

//////////////////////////////////////////////////////////////////////////

// AutoGrownMemIO can be dumped into DataIO

//////////////////////////////////////////////////////////////////////////

} // namespace febird

#endif

