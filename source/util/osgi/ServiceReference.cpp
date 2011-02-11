#include <util/osgi/ServiceReference.h>

#include <string>

using namespace std;

using namespace izenelib::osgi;
using namespace izenelib::osgi::logging;

Logger& ServiceReference::logger = LoggerFactory::getLogger( "Framework" );

ServiceReference::ServiceReference()
{
    logger.log( Logger::LOG_DEBUG, "[ServiceReference#ctor] Default ctor called." );
}

ServiceReference::ServiceReference( const string &name, const Properties &properties, const IService::ConstPtr serv ) : serviceName( name ), props( properties ), service( serv )
{
    logger.log( Logger::LOG_DEBUG, "[ServiceReference#ctor] Called." );
}

ServiceReference::ServiceReference( const ServiceReference& serviceRef )
{
    logger.log( Logger::LOG_DEBUG, "[ServiceReference#copy-ctor] Called." );
    this->serviceName = serviceRef.serviceName;
    this->props = serviceRef.props;
    this->service = serviceRef.service;
}

ServiceReference& ServiceReference::operator=( const ServiceReference &serviceRef )
{
    logger.log( Logger::LOG_DEBUG, "[ServiceReference#operator=] Called." );
    if (this != &serviceRef)
    {
        this->serviceName = serviceRef.serviceName;
        this->props = serviceRef.props;
        this->service = serviceRef.service;
    }
    return *this;
}

ServiceReference::~ServiceReference()
{
    logger.log( Logger::LOG_DEBUG, "[ServiceReference#destructor] Called." );
}

string ServiceReference::getServiceName() const
{
    return this->serviceName;
}

Properties ServiceReference::getServiceProperties() const
{
    return this->props;
}

IService::ConstPtr ServiceReference::getService() const
{
    return this->service;
}

void ServiceReference::setService( IService::ConstPtr serv )
{
    this->service = serv;
}

void ServiceReference::setServiceProperties( const Properties& properties )
{
    this->props = properties;
}

void ServiceReference::setServiceName( const string& name )
{
    this->serviceName = name;
}

string ServiceReference::toString() const
{
    ostringstream refStream;
    refStream << "ServiceReference={";
    refStream << "serviceName=" << this->serviceName << ", ";
    refStream << this->props.toString();
    refStream << "}";
    return refStream.str();
}

