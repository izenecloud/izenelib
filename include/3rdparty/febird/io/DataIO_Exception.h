/* vim: set tabstop=4 : */
#ifndef __febird_io_DataIO_Exception_h__
#define __febird_io_DataIO_Exception_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include "../config.h"
#include <stdexcept>
#include <string>

namespace febird {

class FEBIRD_DLL_EXPORT DataFormatException : public std::exception
{
protected:
	std::string m_message;
public:
	explicit DataFormatException(const char* szMsg = "febird::DataFormatException");
	explicit DataFormatException(const std::string& strMsg);
	virtual ~DataFormatException() throw();

	const char* what() const throw() { return m_message.c_str(); }
};

class FEBIRD_DLL_EXPORT InvalidObjectException : public DataFormatException
{
public:
	explicit InvalidObjectException(const char* szMsg = "febird::InvalidObjectException");
	explicit InvalidObjectException(const std::string& strMsg);
};

// a size value is too large, such as container's size
//
class FEBIRD_DLL_EXPORT SizeValueTooLargeException : public DataFormatException
{
public:
	static void checkSizeValue(size_t value, size_t maxValue);
	SizeValueTooLargeException(size_t value, size_t maxValue, const char* szMsg = "febird::SizeValueTooLargeException");
	explicit SizeValueTooLargeException(const std::string& strMsg);
};

class FEBIRD_DLL_EXPORT BadVersionException : public DataFormatException
{
	unsigned m_loaded_version;
	unsigned m_curr_version;
public:
	explicit BadVersionException(unsigned loaded_version, unsigned curr_version, const char* className);
};

class FEBIRD_DLL_EXPORT NotFoundFactoryException : public DataFormatException
{
public:
	explicit NotFoundFactoryException(const char* szMsg = "febird::NotFoundFactoryException");
	explicit NotFoundFactoryException(const std::string& strMsg);
};


} // namespace febird

#endif // __febird_io_DataIO_Exception_h__
