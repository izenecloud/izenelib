/* vim: set tabstop=4 : */

#include <febird/io/DataIO_Exception.h>
#include <sstream>
#include <stdio.h>

namespace febird {

DataFormatException::DataFormatException(const char* szMsg)
	: m_message(szMsg)
{ }

DataFormatException::DataFormatException(const std::string& strMsg)
	: m_message(strMsg)
{ }

DataFormatException::~DataFormatException() throw()
{}

InvalidObjectException::InvalidObjectException(const char* szMsg)
	: DataFormatException(szMsg)
{ }

InvalidObjectException::InvalidObjectException(const std::string& strMsg)
	: DataFormatException(strMsg)
{ }

// a size value is too large, such as container's size
//
void SizeValueTooLargeException::checkSizeValue(size_t value, size_t maxValue)
{
	if (value > maxValue)
		throw SizeValueTooLargeException(value, maxValue);
}
SizeValueTooLargeException::SizeValueTooLargeException(size_t value, size_t maxValue, const char* szMsg)
	: DataFormatException(szMsg)
{
	char szBuf[256];
	sprintf(szBuf, "[value=%zd(0x%zX), maxValue=%zd(0x%zX)]", value, value, maxValue, maxValue);
	m_message.append(szBuf);
}
SizeValueTooLargeException::SizeValueTooLargeException(const std::string& strMsg)
	: DataFormatException(strMsg)
{ }

BadVersionException::BadVersionException(unsigned loaded_version, unsigned curr_version, const char* className)
	: DataFormatException(""), m_loaded_version(loaded_version), m_curr_version(curr_version)
{
	std::ostringstream oss;
	oss << "class=\"" << className << "\", version[loaded=" << loaded_version << ", current=" << curr_version << "]";
	m_message = oss.str();
}

NotFoundFactoryException::NotFoundFactoryException(const char* szMsg)
	: DataFormatException(szMsg)
{ }
NotFoundFactoryException::NotFoundFactoryException(const std::string& strMsg)
	: DataFormatException(strMsg)
{ }


} // namespace febird

