#ifndef IZENE_UTIL_EXCEPTION_H
#define IZENE_UTIL_EXCEPTION_H

#include <boost/assert.hpp>
#include <boost/exception.hpp>
#include <boost/exception/info.hpp>
#include <exception>

#include <string>
#include <iostream>

using namespace std;
using namespace boost;


NS_IZENELIB_UTIL_BEGIN

typedef string badparamname_info_type;
typedef string badparamvalue_info_type;
typedef string functionname_info_type;
typedef string returncode_info_type;

typedef boost::error_info<struct tag_badparamname, badparamname_info_type> badparamname_info;
typedef boost::error_info<struct tag_badparamvalue, badparamvalue_info_type> badparamvalue_info;
typedef boost::tuple<badparamname_info, badparamvalue_info> badparam_info;

typedef boost::error_info<struct tag_functionname, functionname_info_type> functionname_info;
typedef boost::error_info<struct tag_returncode, returncode_info_type> returncode_info;
typedef boost::tuple<functionname_info, returncode_info> badreturncode_info;


class IZENELIBException : public boost::exception, public std::exception
{
public:
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
    IZENELIBException(ExceptionCode code, string d="") throw()
    {
        string ss = s_errorStrings[code];
        detail = ss+":"+d;
    }
    IZENELIBException(string d="") throw():detail(d) {}

    ~IZENELIBException() throw() {}

public:
    virtual const char* what() const throw ()
    {
        string ret="";

        if (getDetail().compare("")!=0)
            ret += "\n" + getDetail();

        boost::shared_ptr<const char* const> func = boost::get_error_info<boost::throw_function>(*this);
        boost::shared_ptr<const char* const> file = boost::get_error_info<boost::throw_file>(*this);
        boost::shared_ptr<const int> line = boost::get_error_info<boost::throw_line>(*this);
        if (func.get()!=0 && file.get()!=0 && line.get()!=0)
        {
            std::stringstream out;
            out << "\n\tInfo: " << *func << " in file " << *file << "(l. " << *line << ")\n";
            ret += out.str();
        }

        return ret.c_str();
    }

    string getDetail() const throw()
    {
        return detail;
    }

protected:
    string detail;

    static const char* s_errorStrings[NUM_ERRORS];
};

const char* IZENELIBException::s_errorStrings[NUM_ERRORS] =
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


NS_IZENELIB_UTIL_END

#endif //End of IZENE_UTIL_EXCEPTION_H
