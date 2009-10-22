#include <util/Exception.h>

using namespace izenelib::util;

class MyException : public IZENELIBException
{
public:
    enum MyErrorCode
    {
        EC_BADPARAM,               ///<method has been called with bad parameter
        NUM_ERRORS                 ///<no error
    };

    MyException(MyErrorCode ec, string d="")
    {
        string ss = my_errorstring[ec];
        detail = ss+":"+d;
    }
    ~MyException() throw() {}

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

const char* MyException::ece_errorstring[EC_NOERROR] =
{
    "Missing parameter"		//MISSING_PARAMETER_ERROR
};


void f() throw(IZENELIBException)
{
    BOOST_THROW_EXCEPTION(IZENELIBException(IZENELIBException::ERROR_GENERIC,"bad audio frame parameter"));
}
void g()
{
    BOOST_THROW_EXCEPTION(MyException(MyException::EC_BADPARAM,"bad audio frame parameter")
                          <<  badparam_info(badparamname_info("para_name"),
                                            badparamvalue_info("bad_value")) );
}
int main()
{
    try
    {
        //f();
        g();
    }
    catch (IZENELIBException& e)
    {
        cout<<e.what()<<endl;
    }
}

