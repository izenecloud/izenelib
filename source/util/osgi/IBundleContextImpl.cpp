#include <util/osgi/IBundleContextImpl.h>
#include <util/osgi/ServiceInfo.h>
#include <util/osgi/IServiceRegistration.h>
#include <util/osgi/IServiceRegistrationImpl.h>
#include <util/osgi/util/LoggerFactory.h>

using namespace std;
using namespace izenelib::osgi;
using namespace izenelib::osgi::logging;

Logger& IBundleContextImpl::logger_ = LoggerFactory::getLogger( "Framework" );

IBundleContextImpl::IBundleContextImpl( const string& bdleName, IRegistry& reg ) : bundleName_( bdleName ), registry_( reg )
{
    logger_.log( Logger::LOG_DEBUG, "[IBundleContextImpl#ctor] Called, bundle name: %1", bdleName );
}

IBundleContextImpl::~IBundleContextImpl()
{
    logger_.log( Logger::LOG_DEBUG, "[IBundleContextImpl#destructor] Called." );
}

string IBundleContextImpl::getBundleName()
{
    return this->bundleName_;
}

IServiceRegistration* IBundleContextImpl::registerService( const string& className, IService::ConstPtr service, const Properties &dict )
{
    logger_.log( Logger::LOG_DEBUG, "[IBundleContextImpl#registerService] Called, bundle name: %1, service name: %2", this->bundleName_, className );
    ServiceInfoPtr serviceInfo( new ServiceInfo( className, service, dict ) );
    return this->registry_.addServiceInfo( this->bundleName_, serviceInfo );
}

void IBundleContextImpl::addServiceListener( IServiceListener::ConstPtr serviceListener, const string &serviceName )
{
    logger_.log( Logger::LOG_DEBUG, "[IBundleContextImpl#addServiceListener] Called, bundle name: %1, service name: %2", this->bundleName_, serviceName );
    ServiceListenerInfoPtr listenerInfo( new ServiceListenerInfo( bundleName_, serviceName, serviceListener ) );
    this->registry_.addServiceListener( this->bundleName_, listenerInfo );
}

void IBundleContextImpl::removeServiceListener( IServiceListener::ConstPtr serviceListener )
{
    logger_.log( Logger::LOG_DEBUG, "[IBundleContextImpl#removeServiceListener] Called, bundle name: %1", this->bundleName_ );
    ServiceListenerInfoPtr info( new ServiceListenerInfo( bundleName_, "", serviceListener ) );
    this->registry_.removeServiceListener( this->bundleName_, info );
}

