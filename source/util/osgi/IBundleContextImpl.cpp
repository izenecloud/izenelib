#include <util/osgi/IBundleContextImpl.h>
#include <util/osgi/ServiceInfo.h>
#include <util/osgi/IServiceRegistration.h>
#include <util/osgi/IServiceRegistrationImpl.h>
#include <util/osgi/util/LoggerFactory.h>

using namespace std;
using namespace izenelib::osgi;
using namespace izenelib::osgi::logging;

Logger& IBundleContextImpl::logger = LoggerFactory::getLogger( "Framework" );

IBundleContextImpl::IBundleContextImpl( const string& bdleName, IRegistry& reg ) : bundleName( bdleName ), registry( reg )
{
    logger.log( Logger::LOG_DEBUG, "[IBundleContextImpl#ctor] Called, bundle name: %1", bdleName );
}

IBundleContextImpl::~IBundleContextImpl()
{
    logger.log( Logger::LOG_DEBUG, "[IBundleContextImpl#destructor] Called." );
}

string IBundleContextImpl::getBundleName()
{
    return this->bundleName;
}

IServiceRegistration* IBundleContextImpl::registerService( const string& className, IService::ConstPtr service, const Properties &dict )
{
    logger.log( Logger::LOG_DEBUG, "[IBundleContextImpl#registerService] Called, bundle name: %1, service name: %2", this->bundleName, className );
    ServiceInfoPtr serviceInfo( new ServiceInfo( className, service, dict ) );
    return this->registry.addServiceInfo( this->bundleName, serviceInfo );
}

void IBundleContextImpl::addServiceListener( IServiceListener::ConstPtr serviceListener, const string &serviceName )
{
    logger.log( Logger::LOG_DEBUG, "[IBundleContextImpl#addServiceListener] Called, bundle name: %1, service name: %2", this->bundleName, serviceName );
    ServiceListenerInfoPtr listenerInfo( new ServiceListenerInfo( bundleName, serviceName, serviceListener ) );
    this->registry.addServiceListener( this->bundleName, listenerInfo );
}

void IBundleContextImpl::removeServiceListener( IServiceListener::ConstPtr serviceListener )
{
    logger.log( Logger::LOG_DEBUG, "[IBundleContextImpl#removeServiceListener] Called, bundle name: %1", this->bundleName );
    ServiceListenerInfoPtr info( new ServiceListenerInfo( bundleName, "", serviceListener ) );
    this->registry.removeServiceListener( this->bundleName, info );
}

