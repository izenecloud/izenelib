/* vim: set tabstop=4 : */
#ifndef __ConcurrentStream_h__
#define __ConcurrentStream_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include "../stdtypes.h"

#include <boost/thread.hpp>

namespace febird {

template<class StreamClass>
class ConcurrentStreamWrapper : public StreamClass
{
	ConcurrentStreamWrapper(const ConcurrentStreamWrapper&);
	ConcurrentStreamWrapper& operator=(const ConcurrentStreamWrapper&);
protected:
	mutable boost::mutex mutex_;
public:
	ConcurrentStreamWrapper() {}

	template<class T1>
	ConcurrentStreamWrapper(T1 p1) : StreamClass(p1) {}
	template<class T1, class T2>
	ConcurrentStreamWrapper(T1 p1, T2 p2) : StreamClass(p1, p2) {}
	template<class T1, class T2, class T3>
	ConcurrentStreamWrapper(T1 p1, T2 p2, T3 p3) : StreamClass(p1, p2, p3) {}

	int getByte()
	{
		boost::mutex::scoped_lock lock(mutex_);
		return StreamClass::getByte();
	}
	byte readByte()
	{
		boost::mutex::scoped_lock lock(mutex_);
		return StreamClass::readByte();
	}
	void writeByte(byte b) 
	{
		boost::mutex::scoped_lock lock(mutex_);
		return StreamClass::writeByte(b);
	}
	size_t read(void* data, size_t length)
	{
		boost::mutex::scoped_lock lock(mutex_);
		return StreamClass::read(data, length);
	}
	size_t write(const void* data, size_t length)
	{
		boost::mutex::scoped_lock lock(mutex_);
		return StreamClass::write(data, length);
	}
	void flush()
	{
		boost::mutex::scoped_lock lock(mutex_);
		StreamClass::flush();
	}
	bool eof()
	{
		boost::mutex::scoped_lock lock(mutex_);
		return StreamClass::eof();
	}

	bool seek(size_t newPos)
	{
		boost::mutex::scoped_lock lock(mutex_);
		return StreamClass::seek(newPos);
	}
	bool seek(long offset, int origin)
	{
		boost::mutex::scoped_lock lock(mutex_);
		return StreamClass::seek(offset, origin);
	}
	size_t tell()
	{
		boost::mutex::scoped_lock lock(mutex_);
		return StreamClass::tell();
	}	

// for divided dual IO
	bool seekp(size_t newPos)
	{
		boost::mutex::scoped_lock lock(mutex_);
		return StreamClass::seekp(newPos);
	}
	bool seekp(long offset, int origin)
	{
		boost::mutex::scoped_lock lock(mutex_);
		return StreamClass::seekp(offset, origin);
	}
	size_t tellp()
	{
		boost::mutex::scoped_lock lock(mutex_);
		return StreamClass::tellp();
	}
	bool seekg(size_t newPos)
	{
		boost::mutex::scoped_lock lock(mutex_);
		return StreamClass::seekg(newPos);
	}
	bool seekg(long offset, int origin)
	{
		boost::mutex::scoped_lock lock(mutex_);
		return StreamClass::seekg(offset, origin);
	}
	size_t tellg()
	{
		boost::mutex::scoped_lock lock(mutex_);
		return StreamClass::tellg();
	}
};

} // namespace febird

#endif // __ConcurrentStream_h__
