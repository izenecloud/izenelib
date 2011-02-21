#include <util/osgi/IServiceRegistrationImpl.h>

using namespace izenelib::osgi;
using namespace izenelib::osgi::logging;

Logger& IServiceRegistrationImpl::logger_ = LoggerFactory::getLogger( "Framework" );

IServiceRegistrationImpl::IServiceRegistrationImpl( const string& bName, IRegistry* reg, ServiceInfoPtr info ) 
    :registry_( reg ), 
     serviceInfo_( info ),
     bundleName_( bName )
{
    logger_.log( Logger::LOG_DEBUG, "[IServiceRegistrationImpl#ctor] Called." );
}

IServiceRegistrationImpl::~IServiceRegistrationImpl()
{
    logger_.log( Logger::LOG_DEBUG, "[IServiceRegistrationImpl#destructor] Called." );
}

IRegistry* IServiceRegistrationImpl::getRegistry()
{
    return registry_;
}

void IServiceRegistrationImpl::unregister()
{
    logger_.log( Logger::LOG_DEBUG, "[IServiceRegistrationImpl#unregister] Called, service info: %1", this->serviceInfo_->toString() );
    registry_->removeServiceInfo( this->bundleName_, this->serviceInfo_ );
}
