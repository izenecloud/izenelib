#ifndef IZENE_UTIL_EXCEPTION_H
#define IZENE_UTIL_EXCEPTION_H

#include <iostream>
#include <string>
#include <sstream>

using namespace std;


NS_IZENELIB_UTIL_BEGIN

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
    NUM_ERRORS
};

static string s_errorStrings[NUM_ERRORS] =
{
    "Unknown error",			//UNKNOWN_ERROR
    "Generic error",			//GENERIC_ERROR
    "Missing parameter",		//MISSING_PARAMETER_ERROR
    "Bad parameter",			//BAD_PARAMETER_ERROR
    "File I/O error",			//FILEIO_ERROR
    "Rumtime error",			//RUNTIME_ERROR
    "Out of memory",			//OUTOFMEM_ERROR
    "Illegal argument",			//ILLEGALARGUMENT_ERROR
    "Unsupported operation",		//UNSUPPORTED_ERROR
    "Out of range",			//OUTOFRANG_ERROR
};

///IZENELIBException
class IZENELIBException
{
public:
    IZENELIBException(exception_type code,const string& msg = ""):_what(s_errorStrings[code] + ":" + msg),_code(code) {}
    IZENELIBException(exception_type code,const string& msg,const string& file,int line) : _code(code)
    {
        stringstream ss;
        ss << s_errorStrings[code] << ":" << msg << " at file: " << file << ",line:" << line;
        _what = ss.str();
        std::cout << _what << std::endl;
    }
    IZENELIBException(const IZENELIBException& clone):_what(clone._what),_code(clone._code)
    {
    }
    IZENELIBException&operator=(const IZENELIBException& right)
    {
        _code = right._code;
        _what = right._what;
        return *this;
    }
    virtual ~IZENELIBException() {}
    virtual const char* what()
    {
        return _what.c_str();
    }
    virtual exception_type	code()
    {
        return _code;
    }

protected:
    string			_what;
    exception_type	_code;
};

#define IZENELIB_THROW(code,msg) throw izenelib::util::IZENELIBException(code,msg,__FILE__, __LINE__)
#define IZENELIB_RETHROW(e) throw izenelib::util::IZENELIBException(e)

NS_IZENELIB_UTIL_END

#endif //End of IZENE_UTIL_EXCEPTION_H
