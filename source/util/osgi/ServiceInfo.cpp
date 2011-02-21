#include <util/osgi/ServiceInfo.h>

using namespace izenelib::osgi;

Logger& ServiceInfo::logger_ = LoggerFactory::getLogger( "Framework" );

ServiceInfo::ServiceInfo( const string &servName, IService::ConstPtr service, const Properties &properties ) 
    :service_( service ),
     serviceName_( servName ), 
     props_( properties )
{
    logger_.log( Logger::LOG_DEBUG, "[ServiceInfo#ctor] Called." );
}

ServiceInfo::ServiceInfo( const ServiceInfo &serviceInfo )
{
    logger_.log( Logger::LOG_DEBUG, "[ServiceInfo#copy-ctor] Called." );
    this->props_ = serviceInfo.props_;
    this->service_ = serviceInfo.service_;
    this->serviceName_ = serviceInfo.serviceName_;
}


ServiceInfo::~ServiceInfo()
{
    logger_.log( Logger::LOG_DEBUG, "[ServiceInfo#destructor] Called." );
}

string ServiceInfo::getServiceName() const
{
    return this->serviceName_;
}

Properties ServiceInfo::getProperties() const
{
    return this->props_;
}

IService::ConstPtr ServiceInfo::getService() const
{
    return this->service_;
}

ServiceInfo& ServiceInfo::operator=( const ServiceInfo &serviceInfo )
{
    logger_.log( Logger::LOG_DEBUG, "[ServiceInfo#operator=] Called." );

    if (this != &serviceInfo)
    {
        this->service_ = serviceInfo.service_;
        this->serviceName_ = serviceInfo.serviceName_;
        this->props_ = serviceInfo.props_;
    }
    return *this;
}

string ServiceInfo::toString() const
{
    ostringstream propsStream;
    propsStream << "serviceInfo={";
    propsStream << "serviceName_=" << this->serviceName_ << ", ";
    propsStream << this->props_.toString();
    propsStream << "}";
    return propsStream.str();
}

bool ServiceInfo::operator==( const ServiceInfo& serviceInfo1 )
{
    return this->equals( (*this), serviceInfo1 );
}

bool ServiceInfo::equals( const ServiceInfo& info1, const ServiceInfo& info2 )
{
    logger_.log( Logger::LOG_DEBUG, "[ServiceInfo#equals] Called." );
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

