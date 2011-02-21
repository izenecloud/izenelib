#include <util/osgi/ServiceListenerInfo.h>

#include <sstream>

using namespace std;

using namespace izenelib::osgi;

Logger& ServiceListenerInfo::logger_ = LoggerFactory::getLogger( "Framework" );

ServiceListenerInfo::ServiceListenerInfo( const string& bdleName, const string& servName, IServiceListener::ConstPtr serviceListener ) : bundleName_( bdleName ), serviceName_( servName ), serviceListenerObj_( serviceListener )
{
    logger_.log( Logger::LOG_DEBUG, "[ServiceListenerInfo#ctor] Called." );
}

ServiceListenerInfo::~ServiceListenerInfo()
{
    logger_.log( Logger::LOG_DEBUG, "[ServiceListenerInfo#destructor] Called." );
}

ServiceListenerInfo::ServiceListenerInfo( const ServiceListenerInfo& info ) : serviceListenerObj_( info.serviceListenerObj_ )
{
    logger_.log( Logger::LOG_DEBUG, "[ServiceListenerInfo#copy-ctor] Called." );

    this->bundleName_ = info.bundleName_;
    this->serviceName_ = info.serviceName_;
}

ServiceListenerInfo& ServiceListenerInfo::operator=( const ServiceListenerInfo &serviceListenerInfo )
{
    logger_.log( Logger::LOG_DEBUG, "[ServiceListenerInfo#operator=] Called." );

    if (this != &serviceListenerInfo)
    {
        this->bundleName_ = serviceListenerInfo.bundleName_;
        this->serviceName_ = serviceListenerInfo.serviceName_;
        this->serviceListenerObj_ = serviceListenerInfo.serviceListenerObj_;
    }
    return *this;
}

string ServiceListenerInfo::getBundleName() const
{
    return this->bundleName_;
}

string ServiceListenerInfo::getServiceName() const
{
    return this->serviceName_;
}

IServiceListener::ConstPtr ServiceListenerInfo::getServiceListenerObj() const
{
    return this->serviceListenerObj_;
}

bool ServiceListenerInfo::operator==( const ServiceListenerInfo& info1 )
{
    return this->equals( info1, (*this) );
}

bool ServiceListenerInfo::equals( const ServiceListenerInfo& info1, const ServiceListenerInfo& info2 )
{
    if ( info2.getBundleName() == info1.getBundleName() &&
            info2.getServiceListenerObj() == info1.getServiceListenerObj() &&
            info2.getServiceName() == info1.getServiceName() )
    {
        return true;
    }
    else
    {
        return false;
    }
}

string ServiceListenerInfo::toString() const
{
    ostringstream infoStream;
    infoStream << "serviceListenerInfo={";
    infoStream << "bundleName_=" << this->bundleName_ << ", ";
    infoStream << "serviceName_=" << this->serviceName_;
    infoStream << "}";
    return infoStream.str();
}

