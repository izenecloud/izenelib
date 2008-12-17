#ifndef SDBEXCEPTION_H_
#define SDBEXCEPTION_H_

#include <util/Exception.h>

NS_IZENELIB_AM_BEGIN

class SDBException:public 
{
public:
	SDBException(const string& error) :
		message(error) {
	}
	string what() {
		return message;
	}
private:
	string message;
};

class BTreeFileException : public IZENELIBException
{
public:
    enum MyErrorCode
    {
        EC_BADPARAM,               ///<method has been called with bad parameter
        NUM_ERRORS,                 ///<no error
        KEY_MISMATCH
    };

    BTreeFileException(MyErrorCode ec, string d="")
    {
        string ss = my_errorstring[ec];
        detail = ss+":"+d;
    }
    ~BTreeFileException() throw() {}

    const char* what() const throw ()
    {
        string ret = BaseException::what();
        if (boost::shared_ptr<const string>   badparamname = boost::get_error_info<badparamname_info>(*this) )
        {
            std::stringstream out;
            out << *badparamname;
            ret += out.str();
        }

        if (boost::shared_ptr<const string>   badparam = boost::get_error_info<badparamvalue_info>(*this) )
        {
            std::stringstream out;
            out << *badparam;
            ret += out.str();
        }
        return ret.c_str();
    }

private:
    static const char* my_errorstring[EC_NOERROR];
};

const char* BTreeFileException::ece_errorstring[EC_NOERROR] =
{
    "Missing parameter",		//MISSING_PARAMETER_ERROR
	"Num_error",
	"Key, mismatch"
};


NS_IZENELIB_AM_END
#endif /*SBEXCEPTION_H_*/
