/**
* @file        Exception.h
* @author     Yingfeng Zhang
* @version     SF1 v5.0
* @brief The definition of all IndexManagerException
*/
#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <types.h>

#include <iostream>
#include <string>
#include <sstream>

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

typedef unsigned short exception_type;

enum ExceptionCode
{
    ERROR_UNKNOWN,
    ERROR_GENERIC,
    ERROR_MISSING_PARAMETER,
    ERROR_BAD_PARAMETER,
    ERROR_FILEIO,
    ERROR_RUNTIME,
    ERROR_OUTOFMEM,
    ERROR_ILLEGALARGUMENT,
    ERROR_UNSUPPORTED,
    ERROR_OUTOFRANGE,
    ERROR_INDEX_COLLAPSE,
    ERROR_VERSION,
    ERROR_ASSERT,
    ERROR_NETWORK,
    ERROR_DATANODE,
    NUM_ERRORS
};

///IndexManagerException
class IndexManagerException
{
public:
    IndexManagerException(exception_type code,const std::string& msg = ""):_what(s_errorStrings[code] + ":" + msg),_code(code) {}
    IndexManagerException(exception_type code,const std::string& msg,const std::string& file,int line) : _code(code)
    {
        std::stringstream ss;
        ss << s_errorStrings[code] << ":" << msg << " at file: " << file << ",line:" << line;
        _what = ss.str();
        std::cout << _what << std::endl;
    }
    IndexManagerException(const IndexManagerException& clone):_what(clone._what),_code(clone._code)
    {
    }
    IndexManagerException&operator=(const IndexManagerException& right)
    {
        _code = right._code;
        _what = right._what;
        return *this;
    }
    virtual ~IndexManagerException() {}
    virtual const char* what()
    {
        return _what.c_str();
    }
    virtual exception_type	code()
    {
        return _code;
    }

protected:
    static std::string 		s_errorStrings[NUM_ERRORS];
    std::string			_what;
    exception_type	_code;
};
///IllegalArgumentException
class IllegalArgumentException : public IndexManagerException
{
public:
    IllegalArgumentException(const std::string& msg):IndexManagerException(ERROR_ILLEGALARGUMENT,msg)
    {
    }
    ~IllegalArgumentException() {}
};
///UnsupportedOperationException
class UnsupportedOperationException : public IndexManagerException
{
public:
    UnsupportedOperationException(const std::string& msg):IndexManagerException(ERROR_UNSUPPORTED,msg)
    {
    }
    ~UnsupportedOperationException() {}
};
///FileIOException
class FileIOException : public IndexManagerException
{
public:
    FileIOException(const std::string& msg):IndexManagerException(ERROR_FILEIO,msg)
    {
    }
    ~FileIOException() {}
};

///OutOfMemoryException
class OutOfMemoryException : public IndexManagerException
{
public:
    OutOfMemoryException (const std::string& msg) : IndexManagerException(ERROR_OUTOFMEM,msg) {}
    ~OutOfMemoryException() {}
};
///NetworkException
class NetworkException : public IndexManagerException
{
public:
    NetworkException (const std::string& msg) : IndexManagerException(ERROR_NETWORK,msg) {}
    ~NetworkException() {}
};

#define SF1V5_THROW(code,msg) throw izenelib::ir::indexmanager::IndexManagerException(code,msg,__FILE__, __LINE__)
#define SF1V5_RETHROW(e) throw izenelib::ir::indexmanager::IndexManagerException(e)


}


NS_IZENELIB_IR_END

#endif
