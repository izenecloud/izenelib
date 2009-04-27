/* vim: set tabstop=4 : */
#ifndef __ConcurrentStream_h__
#define __ConcurrentStream_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include "../stdtypes.h"

#include "../thread/lockable.h"
#include "../thread/LockSentry.h"

namespace febird {

template<class StreamClass>
class ConcurrentStreamWrapper : public StreamClass
{
	ConcurrentStreamWrapper(const ConcurrentStreamWrapper&);
	ConcurrentStreamWrapper& operator=(const ConcurrentStreamWrapper&);
protected:
	thread::MutexLock lock;
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
		thread::MutexLock::Sentry sentry(lock);
		return StreamClass::getByte();
	}
	byte readByte()
	{
		thread::MutexLock::Sentry sentry(lock);
		return StreamClass::readByte();
	}
	void writeByte(byte b) 
	{
		thread::MutexLock::Sentry sentry(lock);
		return StreamClass::writeByte(b);
	}
	size_t read(void* data, size_t length)
	{
		thread::MutexLock::Sentry sentry(lock);
		return StreamClass::read(data, length);
	}
	size_t write(const void* data, size_t length)
	{
		thread::MutexLock::Sentry sentry(lock);
		return StreamClass::write(data, length);
	}
	void flush()
	{
		thread::MutexLock::Sentry sentry(lock);
		StreamClass::flush();
	}
	bool eof()
	{
		thread::MutexLock::Sentry sentry(lock);
		return StreamClass::eof();
	}

	bool seek(size_t newPos)
	{
		thread::MutexLock::Sentry sentry(lock);
		return StreamClass::seek(newPos);
	}
	bool seek(long offset, int origin)
	{
		thread::MutexLock::Sentry sentry(lock);
		return StreamClass::seek(offset, origin);
	}
	size_t tell()
	{
		thread::MutexLock::Sentry sentry(lock);
		return StreamClass::tell();
	}	

// for divided dual IO
	bool seekp(size_t newPos)
	{
		thread::MutexLock::Sentry sentry(lock);
		return StreamClass::seekp(newPos);
	}
	bool seekp(long offset, int origin)
	{
		thread::MutexLock::Sentry sentry(lock);
		return StreamClass::seekp(offset, origin);
	}
	size_t tellp()
	{
		thread::MutexLock::Sentry sentry(lock);
		return StreamClass::tellp();
	}
	bool seekg(size_t newPos)
	{
		thread::MutexLock::Sentry sentry(lock);
		return StreamClass::seekg(newPos);
	}
	bool seekg(long offset, int origin)
	{
		thread::MutexLock::Sentry sentry(lock);
		return StreamClass::seekg(offset, origin);
	}
	size_t tellg()
	{
		thread::MutexLock::Sentry sentry(lock);
		return StreamClass::tellg();
	}
};

} // namespace febird

#endif // __ConcurrentStream_h__
