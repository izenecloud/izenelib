#include <util/osgi/ServiceReference.h>

#include <string>

using namespace std;

using namespace izenelib::osgi;
using namespace izenelib::osgi::logging;

Logger& ServiceReference::logger_ = LoggerFactory::getLogger( "Framework" );

ServiceReference::ServiceReference()
{
    logger_.log( Logger::LOG_DEBUG, "[ServiceReference#ctor] Default ctor called." );
}

ServiceReference::ServiceReference( const string &name, const Properties &properties, const IService::ConstPtr serv ) : serviceName_( name ), props_( properties ), service_( serv )
{
    logger_.log( Logger::LOG_DEBUG, "[ServiceReference#ctor] Called." );
}

ServiceReference::ServiceReference( const ServiceReference& serviceRef )
{
    logger_.log( Logger::LOG_DEBUG, "[ServiceReference#copy-ctor] Called." );
    this->serviceName_ = serviceRef.serviceName_;
    this->props_ = serviceRef.props_;
    this->service_ = serviceRef.service_;
}

ServiceReference& ServiceReference::operator=( const ServiceReference &serviceRef )
{
    logger_.log( Logger::LOG_DEBUG, "[ServiceReference#operator=] Called." );
    if (this != &serviceRef)
    {
        this->serviceName_ = serviceRef.serviceName_;
        this->props_ = serviceRef.props_;
        this->service_ = serviceRef.service_;
    }
    return *this;
}

ServiceReference::~ServiceReference()
{
    logger_.log( Logger::LOG_DEBUG, "[ServiceReference#destructor] Called." );
}

string ServiceReference::getServiceName() const
{
    return this->serviceName_;
}

Properties ServiceReference::getServiceProperties() const
{
    return this->props_;
}

IService::ConstPtr ServiceReference::getService() const
{
    return this->service_;
}

void ServiceReference::setService( IService::ConstPtr serv )
{
    this->service_ = serv;
}

void ServiceReference::setServiceProperties( const Properties& properties )
{
    this->props_ = properties;
}

void ServiceReference::setServiceName( const string& name )
{
    this->serviceName_ = name;
}

string ServiceReference::toString() const
{
    ostringstream refStream;
    refStream << "ServiceReference={";
    refStream << "serviceName_=" << this->serviceName_ << ", ";
    refStream << this->props_.toString();
    refStream << "}";
    return refStream.str();
}

