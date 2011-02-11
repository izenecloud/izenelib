#include <util/osgi/ConfigurationException.h>

using namespace izenelib::osgi;

ConfigurationException::ConfigurationException( const std::string &msg ) : OSGIException( msg )
{

}

ConfigurationException::~ConfigurationException() throw()
{

}

