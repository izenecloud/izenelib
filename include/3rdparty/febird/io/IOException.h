/* vim: set tabstop=4 : */
#ifndef __febird_io_IOException_h__
#define __febird_io_IOException_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <exception>
#include <string>
#include "../config.h"

namespace febird {

class FEBIRD_DLL_EXPORT IOException : public std::exception
{
protected:
	int m_errCode;
	std::string m_message;
public:
	explicit IOException(const char* szMsg = "febird::IOException");
	explicit IOException(int errCode, const char* szMsg = "febird::IOException");
	virtual ~IOException() throw() {}

	const char* what() const throw() { return m_message.c_str(); }
	int errCode() const throw() { return m_errCode; }

	static int lastError();
	static std::string errorText(int errCode);
};

class FEBIRD_DLL_EXPORT OpenFileException : public IOException
{
	std::string m_path;
public:
	explicit OpenFileException(const char* path, const char* szMsg = "febird::OpenFileException");
	~OpenFileException() throw() {}
};

// blocked streams read 0 bytes will cause this exception
// other streams read not enough maybe cause this exception
// all streams read 0 bytes will cause this exception
class FEBIRD_DLL_EXPORT EndOfFileException : public IOException
{
public:
	explicit EndOfFileException(const char* szMsg = "febird::EndOfFileException")
		: IOException(szMsg)
	{ }
};

class FEBIRD_DLL_EXPORT OutOfSpaceException : public IOException
{
public:
	explicit OutOfSpaceException(const char* szMsg = "febird::OutOfSpaceException")
		: IOException(szMsg)
	{ }
};

class FEBIRD_DLL_EXPORT DelayedWriteFailException : public IOException
{
public:
	DelayedWriteFailException(const char* szMsg = "febird::DelayedWriteFailException")
		: IOException(szMsg)
	{ }
//	size_t streamPosition;
};


} // namespace febird

#endif // __febird_io_IOException_h__
