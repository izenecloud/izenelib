#include <util/osgi/ObjectCreationException.h>

using namespace izenelib::osgi;

ObjectCreationException::ObjectCreationException( const std::string &msg ) : message( msg )
{
}

ObjectCreationException::~ObjectCreationException( ) throw()
{
}

const char* ObjectCreationException::what() const throw()
{
    return this->message.c_str();
}
