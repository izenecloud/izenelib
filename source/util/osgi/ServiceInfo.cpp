#include <util/osgi/ServiceInfo.h>

using namespace izenelib::osgi;

Logger& ServiceInfo::logger = LoggerFactory::getLogger( "Framework" );

ServiceInfo::ServiceInfo( const string &servName, IService::ConstPtr service, const Properties &properties ) 
    :service( service ),
     serviceName( servName ), 
     props( properties )
{
    logger.log( Logger::LOG_DEBUG, "[ServiceInfo#ctor] Called." );
}

ServiceInfo::ServiceInfo( const ServiceInfo &serviceInfo )
{
    logger.log( Logger::LOG_DEBUG, "[ServiceInfo#copy-ctor] Called." );
    this->props = serviceInfo.props;
    this->service = serviceInfo.service;
    this->serviceName = serviceInfo.serviceName;
}


ServiceInfo::~ServiceInfo()
{
    logger.log( Logger::LOG_DEBUG, "[ServiceInfo#destructor] Called." );
}

string ServiceInfo::getServiceName() const
{
    return this->serviceName;
}

Properties ServiceInfo::getProperties() const
{
    return this->props;
}

IService::ConstPtr ServiceInfo::getService() const
{
    return this->service;
}

ServiceInfo& ServiceInfo::operator=( const ServiceInfo &serviceInfo )
{
    logger.log( Logger::LOG_DEBUG, "[ServiceInfo#operator=] Called." );

    if (this != &serviceInfo)
    {
        this->service = serviceInfo.service;
        this->serviceName = serviceInfo.serviceName;
        this->props = serviceInfo.props;
    }
    return *this;
}

string ServiceInfo::toString() const
{
    ostringstream propsStream;
    propsStream << "serviceInfo={";
    propsStream << "serviceName=" << this->serviceName << ", ";
    propsStream << this->props.toString();
    propsStream << "}";
    return propsStream.str();
}

bool ServiceInfo::operator==( const ServiceInfo& serviceInfo1 )
{
    return this->equals( (*this), serviceInfo1 );
}

bool ServiceInfo::equals( const ServiceInfo& info1, const ServiceInfo& info2 )
{
    logger.log( Logger::LOG_DEBUG, "[ServiceInfo#equals] Called." );
    if ( info1.getServiceName() == info2.getServiceName() &&
            info1.getProperties() == info2.getProperties() &&
            info1.getService() == info2.getService() )
    {
        return true;
    }
    else
    {
        return false;
    }
}

