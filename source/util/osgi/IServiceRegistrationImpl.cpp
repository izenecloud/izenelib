#include <util/osgi/IServiceRegistrationImpl.h>

using namespace izenelib::osgi;
using namespace izenelib::osgi::logging;

Logger& IServiceRegistrationImpl::logger = LoggerFactory::getLogger( "Framework" );

IServiceRegistrationImpl::IServiceRegistrationImpl( const string& bName, IRegistry& reg, ServiceInfoPtr info ) 
    :registry( reg ), 
     serviceInfo( info ),
     bundleName( bName )
{
    logger.log( Logger::LOG_DEBUG, "[IServiceRegistrationImpl#ctor] Called." );
}

IServiceRegistrationImpl::~IServiceRegistrationImpl()
{
    logger.log( Logger::LOG_DEBUG, "[IServiceRegistrationImpl#destructor] Called." );
}

void IServiceRegistrationImpl::unregister()
{
    logger.log( Logger::LOG_DEBUG, "[IServiceRegistrationImpl#unregister] Called, service info: %1", this->serviceInfo->toString() );
    this->registry.removeServiceInfo( this->bundleName, this->serviceInfo );
}
