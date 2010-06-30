#ifndef IZENE_UTIL_EXCEPTION_H
#define IZENE_UTIL_EXCEPTION_H

#include <boost/assert.hpp>
#include <boost/version.hpp>
#if BOOST_VERSION >= 104000
# include <boost/exception/all.hpp>
#else
# include <boost/exception.hpp>
#endif
#include <boost/exception/info.hpp>
#include <exception>
#include <boost/throw_exception.hpp>

#include <string>
#include <iostream>


NS_IZENELIB_UTIL_BEGIN

typedef std::string badparamname_info_type;
typedef std::string badparamvalue_info_type;
typedef std::string functionname_info_type;
typedef std::string returncode_info_type;

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
    IZENELIBException(ExceptionCode code,const std::string& d="") throw():detail(d) {}

    IZENELIBException(const std::string& d="") throw():detail(d) {}

    ~IZENELIBException() throw() {}

public:
    virtual const char* what() const throw ()
    {
//         std::string ret="";
// 
//         if (getDetail().compare("")!=0)
//             ret += "\n" + getDetail();
// 
//         boost::shared_ptr<const char* const> func = boost::get_error_info<boost::throw_function>(*this);
//         boost::shared_ptr<const char* const> file = boost::get_error_info<boost::throw_file>(*this);
//         boost::shared_ptr<const int> line = boost::get_error_info<boost::throw_line>(*this);
//         if (func.get()!=0 && file.get()!=0 && line.get()!=0)
//         {
//             std::stringstream out;
//             out << "\t[Info: " << *func << " in file " << *file << "(l. " << *line << ")]";
//             ret += out.str();
//         }
// 
//         return ret.c_str();
        return getDetail().c_str();
    }

    std::string getDetail() const throw()
    {
        return detail;
    }

protected:
    std::string detail;
};


NS_IZENELIB_UTIL_END
#define IZENELIB_THROW(msg) throw( izenelib::util::IZENELIBException(msg) )
#endif //End of IZENE_UTIL_EXCEPTION_H

