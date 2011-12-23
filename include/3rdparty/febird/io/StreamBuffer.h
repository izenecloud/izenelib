/* vim: set tabstop=4 : */
#ifndef __febird_io_StreamBuffer_h__
#define __febird_io_StreamBuffer_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include "IOException.h"
#include "IStream.h"
#include "MemStream.h"

#include "../refcount.h"

/**
 @file 以 Stream 的 Concept, 实现的 Buffer

 可以和 Stream 一样的用，但提供了高速缓冲——是 C FILE 的大约 20 倍

 - 使用缓冲倒置：
   -# 一般的实现(BufferStream)是给 Stream 增加缓冲功能
   -# 而这个实现(StreamBuffer)是给缓冲增加 Stream 功能
   -# 这里的主体是 Buffer, 而非 Stream

 - 高性能主要通过以下几点技法实现
   -# 让调用最频繁的函数 inline, 并保持函数内容最短, 如 read/ensureRead/readByte, 等
   -# 在这些 inline 函数最频繁的执行路径上, 执行最少的代码 @see InputBuffer::readByte
   -# 只有极少情况下会执行的分支, 封装到一个非虚函数(非虚函数的调用代码比虚函数小)
   -# 如此, inline 函数的执行效率会非常高, 在 Visual Studio 下, ensureRead 中的 memcpy 在大多数情况下完全被优化掉了:
	 @code
	  LittleEndianDataInput<InputBuffer> input(&buf);
	  int x;
	  input >> x;
	  // 在这个代码段中, input >> x;
	  // 的最频繁分支甚至被优化成了等价代码: x = *(int*)m_cur;
	  // 底层函数调用的 memcpy 完全被优化掉了
	 @endcode

 - 共有五种 StreamBuffer: InputBuffer/OutputBuffer/SeekableInputBuffer/SeekableOutputBuffer/SeekableBuffer
   -# 每种 buffer 都可以附加(attach)一个支持相应功能的流
   -# SeekableInputBuffer 并不要求 stream 必须是 ISeekableInputStream,
      只需要 stream 同时实现了 ISeekable 和 IInputStream 即可
   -# SeekableBuffer 并不要求 stream 是 ISeekableStream,
      只需要 stream 同时实现了 ISeekable/IInputStream/IOutputStream
 */

namespace febird {

class FEBIRD_DLL_EXPORT IOBufferBase : public RefCounter
{
private:
	// can not copy
	IOBufferBase(const IOBufferBase&);
	const IOBufferBase& operator=(const IOBufferBase&);

public:
	IOBufferBase();
	virtual ~IOBufferBase();

	//! 设置 buffer 尺寸并分配 buffer 内存
	//! 在整个生存期内只能调用一次
	void initbuf(size_t capacity) FEBIRD_RESTRICT;

	//! 如果在 init 之前调用，仅设置 buffer 尺寸
	//! 否则重新分配 buffer 并设置相应的指针
	void set_bufsize(size_t size) FEBIRD_RESTRICT;

	byte*  bufbeg() const { return m_beg; }
	byte*  bufcur() const { return m_cur; }
	byte*  bufend() const { return m_end; }

	size_t bufpos()  const { return m_cur-m_beg; }
	size_t bufsize() const { return m_end-m_beg; }
	size_t bufcapacity() const { return m_capacity; }

	//! only seek in buffer
	//!
	//! when dest stream is null, can seek and used as a memstream
	virtual void seek_cur(ptrdiff_t diff);

	//! set buffer eof
	//!
	//! most for m_is/m_os == 0
	void set_bufeof(size_t eofpos);

protected:
	//! 当调用完 stream.read/write 时，使用该函数来同步内部 pos 变量
	//!
	//! 对 non-seekable stream, 这个函数是空, SeekableBufferBase 改写了该函数
	//! @see SeekableBufferBase::update_pos
	virtual void update_pos(size_t inc) {} // empty for non-seekable

protected:
	// dummy, only for OutputBufferBase::attach to use
	void attach(void*) { }

protected:
	// for  InputBuffer, [m_beg, m_cur) is readed,  [m_cur, m_end) is prefetched
	// for OutputBuffer, [m_beg, m_cur) is written, [m_cur, m_end) is undefined

	byte*  FEBIRD_RESTRICT m_beg;	// buffer ptr
	byte*  FEBIRD_RESTRICT m_cur;	// current read/write position
	byte*  FEBIRD_RESTRICT m_end;   // end mark, m_end <= m_beg + m_capacity && m_end >= m_beg
	size_t m_capacity; // buffer capacity
};

class FEBIRD_DLL_EXPORT InputBuffer : public IOBufferBase
{
public:
//	typedef IInputStream stream_t;
	typedef boost::mpl::false_ is_seekable;

	explicit InputBuffer(IInputStream* stream = 0)
		: m_is(stream)
	{
	}

	void attach(IInputStream* stream)
	{
		m_is = stream;
	}

	bool eof() const { return m_cur == m_end && (0 == m_is || m_is->eof()); }

	size_t read(void* FEBIRD_RESTRICT vbuf, size_t length) FEBIRD_RESTRICT
	{
		if (m_cur+length <= m_end) {
			memcpy(vbuf, m_cur, length);
			m_cur += length;
			return length;
		} else
			return fill_and_read(vbuf, length);
	}
	void ensureRead(void* FEBIRD_RESTRICT vbuf, size_t length) FEBIRD_RESTRICT
	{
		// 为了效率，这么实现可以让编译器更好地 inline 这个函数
		// inline 后的函数体并尽可能小
		if (m_cur+length <= m_end) {
			memcpy(vbuf, m_cur, length);
			m_cur += length;
		} else
			fill_and_ensureRead(vbuf, length);
	}

	byte readByte() FEBIRD_RESTRICT
	{
		if (m_cur < m_end)
			return *m_cur++;
		else
			return fill_and_read_byte();
	}
	int getByte() FEBIRD_RESTRICT
	{
		if (m_cur < m_end)
			return *m_cur++;
		else
			return this->fill_and_get_byte();
	}

	void getline(std::string& line, size_t maxlen);

	template<class OutputStream>
	void to_output(OutputStream& output, size_t length) FEBIRD_RESTRICT
	{
		size_t total = 0;
		while (total < length)
		{
			using namespace std; // for min
			if (m_cur == m_end)
				this->fill_and_read(m_beg, m_end-m_beg);
			size_t nWrite = min(size_t(m_end-m_cur), size_t(length-total));
			output.ensureWrite(m_cur, nWrite);
			total += nWrite;
			m_cur += nWrite;
		}
	}

protected:
	size_t fill_and_read(void* FEBIRD_RESTRICT vbuf, size_t length) FEBIRD_RESTRICT;
	void   fill_and_ensureRead(void* FEBIRD_RESTRICT vbuf, size_t length) FEBIRD_RESTRICT;
	byte   fill_and_read_byte() FEBIRD_RESTRICT;
	int    fill_and_get_byte() FEBIRD_RESTRICT;
	size_t read_min_max(void* FEBIRD_RESTRICT vbuf, size_t min_length, size_t max_length) FEBIRD_RESTRICT;

	virtual size_t do_fill_and_read(void* FEBIRD_RESTRICT vbuf, size_t length) FEBIRD_RESTRICT;

protected:
	IInputStream* m_is;
};

template<class BaseClass>
class FEBIRD_DLL_EXPORT OutputBufferBase : public BaseClass
{
public:
	typedef boost::mpl::false_ is_seekable;

	explicit OutputBufferBase(IOutputStream* os = 0) : m_os(os)
	{
	}
	virtual ~OutputBufferBase();

	template<class Stream>
	void attach(Stream* stream)
	{
		BaseClass::attach(stream);
		m_os = stream;
	}

	void flush();

	size_t write(const void* FEBIRD_RESTRICT vbuf, size_t length) FEBIRD_RESTRICT
	{
		if (m_cur+length <= m_end) {
			memcpy(m_cur, vbuf, length);
			m_cur += length;
			return length;
		} else
			return flush_and_write(vbuf, length);
	}

	void ensureWrite(const void* FEBIRD_RESTRICT vbuf, size_t length) FEBIRD_RESTRICT
	{
		// 为了效率，这么实现可以让编译器更好地 inline 这个函数
		// inline 后的函数体并尽可能小
		if (m_cur+length <= m_end) {
			memcpy(m_cur, vbuf, length);
			m_cur += length;
		} else
			flush_and_ensureWrite(vbuf, length);
	}

	void writeByte(byte b) FEBIRD_RESTRICT
	{
		if (m_cur < m_end)
			*m_cur++ = b;
		else
			flush_and_write_byte(b);
	}

	template<class InputStream>
	void from_input(InputStream& input, size_t length) FEBIRD_RESTRICT
	{
		size_t total = 0;
		while (total < length)
		{
			using namespace std; // for min
			if (m_cur == m_end)
				flush_buffer();
			size_t nRead = min(size_t(m_end-m_cur), size_t(length-total));
			input.ensureRead(m_cur, nRead);
			total += nRead;
			m_cur += nRead;
		}
	}

protected:
	size_t flush_and_write(const void* FEBIRD_RESTRICT vbuf, size_t length) FEBIRD_RESTRICT;
	void   flush_and_ensureWrite(const void* FEBIRD_RESTRICT vbuf, size_t length) FEBIRD_RESTRICT;
	void   flush_and_write_byte(byte b) FEBIRD_RESTRICT;

	virtual size_t do_flush_and_write(const void* FEBIRD_RESTRICT vbuf, size_t length) FEBIRD_RESTRICT;

	virtual void flush_buffer(); // only write to m_os, not flush m_os

protected:
	IOutputStream* m_os;
	using BaseClass::m_cur;
	using BaseClass::m_beg;
	using BaseClass::m_end;
	using BaseClass::m_capacity;
};
typedef OutputBufferBase<IOBufferBase> OutputBuffer;

template<class BaseClass>
class FEBIRD_DLL_EXPORT SeekableBufferBase : public BaseClass
{
protected:
	using BaseClass::m_beg;
	using BaseClass::m_cur;
	using BaseClass::m_end;
	using BaseClass::m_capacity;

public:
	typedef boost::mpl::true_ is_seekable;

	//! constructor
	//!
	//! 如果以 append 方式打开流，这个 m_stream_pos 是不对的
	//! 不过一般这种情况下很少会调用 seek/tell
	//! 如果真这么做，会导致未定义行为
	explicit SeekableBufferBase()
	{
		m_seekable = 0;
		m_stream_pos = 0;
	}

	template<class Stream>
	void attach(Stream* stream)
	{
		BaseClass::attach(stream);
		m_seekable = stream;
	}

	void seek(stream_position_t pos);
	void seek(stream_offset_t offset, int origin);

	void seek_cur(ptrdiff_t diff);

	stream_position_t tell() const;
	stream_position_t size() const;

protected:
	virtual void update_pos(size_t inc); //!< override
	virtual void invalidate_buffer() = 0;

	//! 如果已预取，m_stream_pos 对应缓冲区末尾 m_end
	//! 否则 m_stream_pos 对应缓冲区开始
	virtual int is_prefetched() const = 0;

protected:
	ISeekable* m_seekable;
	stream_position_t m_stream_pos;
};

class FEBIRD_DLL_EXPORT SeekableInputBuffer : public SeekableBufferBase<InputBuffer>
{
	typedef SeekableBufferBase<InputBuffer> super;
public:
	SeekableInputBuffer() { }
protected:
	virtual void invalidate_buffer();
	virtual int is_prefetched() const;
};

class FEBIRD_DLL_EXPORT SeekableOutputBuffer : public SeekableBufferBase<OutputBuffer>
{
	typedef SeekableBufferBase<OutputBuffer> super;

public:
//	typedef boost::mpl::true_ is_seekable;

	//! constructor
	//!
	//! 如果以 append 方式打开流，这个 m_stream_pos 是不对的
	//! 不过一般这种情况下很少会调用 seek/tell
	//! 如果真这么做，会导致未定义行为
	SeekableOutputBuffer() {}

protected:
	virtual void invalidate_buffer();
	virtual int is_prefetched() const;
};

class FEBIRD_DLL_EXPORT SeekableBuffer :
	public SeekableBufferBase<OutputBufferBase<InputBuffer> >
{
	typedef SeekableBufferBase<OutputBufferBase<InputBuffer> > super;

public:
	SeekableBuffer();
	~SeekableBuffer();

	size_t read(void* FEBIRD_RESTRICT vbuf, size_t length) FEBIRD_RESTRICT
	{
		if (m_cur+length <= m_end && m_prefetched) {
			memcpy(vbuf, m_cur, length);
			m_cur += length;
			return length;
		} else
			return fill_and_read(vbuf, length);
	}
	void ensureRead(void* FEBIRD_RESTRICT vbuf, size_t length) FEBIRD_RESTRICT
	{
		// 为了效率，这么实现可以让编译器更好地 inline 这个函数
		// inline 后的函数体并尽可能小
		if (m_cur+length <= m_end && m_prefetched) {
			memcpy(vbuf, m_cur, length);
			m_cur += length;
		} else
			fill_and_ensureRead(vbuf, length);
	}

	byte readByte() FEBIRD_RESTRICT 
	{
		if (m_cur < m_end && m_prefetched)
			return *m_cur++;
		else
			return fill_and_read_byte();
	}
	int getByte() FEBIRD_RESTRICT 
	{
		if (m_cur < m_end && m_prefetched)
			return *m_cur++;
		else
			return fill_and_get_byte();
	}

	size_t write(const void* vbuf, size_t length) FEBIRD_RESTRICT 
	{
		m_dirty = true;
		return super::write(vbuf, length);
	}

	void ensureWrite(const void* vbuf, size_t length) FEBIRD_RESTRICT 
	{
		m_dirty = true;
		super::ensureWrite(vbuf, length);
	}

	void writeByte(byte b) FEBIRD_RESTRICT 
	{
		m_dirty = true;
		super::writeByte(b);
	}

protected:
	virtual size_t do_fill_and_read(void* FEBIRD_RESTRICT vbuf, size_t length) FEBIRD_RESTRICT ; //!< override
	virtual size_t do_flush_and_write(const void* FEBIRD_RESTRICT vbuf, size_t length) FEBIRD_RESTRICT ; //!< override

	virtual void flush_buffer(); //!< override
	virtual void invalidate_buffer(); //!< override
	virtual int is_prefetched() const;

private:
	int m_dirty;
	int m_prefetched;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class FEBIRD_DLL_EXPORT FileStreamBuffer : public SeekableBuffer
{
public:
	explicit FileStreamBuffer(const char* FEBIRD_RESTRICT fname, const char* FEBIRD_RESTRICT mode, size_t capacity = 8*1024);
	~FileStreamBuffer();
};

} // febird

#endif // __febird_io_StreamBuffer_h__


