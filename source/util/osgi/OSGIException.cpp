#include <util/osgi/OSGIException.h>

using namespace izenelib::osgi;

OSGIException::OSGIException( const std::string &msg ) : message( msg )
{

}

OSGIException::~OSGIException( ) throw()
{
}

const char* OSGIException::what() const throw()
{
    return this->message.c_str();
}

