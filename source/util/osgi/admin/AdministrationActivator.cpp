#include <util/osgi/Properties.h>
#include <util/osgi/ObjectCreator.h>
#include <util/osgi/NullCreator.h>

#include <util/osgi/admin/AdministrationActivator.h>
#include <util/osgi/admin/IAdministrationServiceImpl.h>

using namespace izenelib::osgi;
using namespace izenelib::osgi::admin;
using namespace izenelib::osgi::logging;

Logger& AdministrationActivator::log = LoggerFactory::getLogger( "services" );

AdministrationActivator::AdministrationActivator()
{
    log.log( Logger::LOG_DEBUG, "[AdministrationActivator#ctor] Called." );
    this->adminProvider = 0;
    this->adminService = 0;
    this->serviceReg = 0;
}

AdministrationActivator::~AdministrationActivator()
{
    if(! this->serviceReg)
        delete this->serviceReg;
    if(! this->adminService)
        delete this->adminService;
}

void AdministrationActivator::start( IBundleContext::ConstPtr context )
{
    log.log( Logger::LOG_DEBUG, "[AdministrationActivator#start] Called." );
    Properties props;
    this->adminService = new IAdministrationServiceImpl( this->adminProvider );
    this->serviceReg = context->registerService( "sof::services::admin::IAdministrationService", this->adminService, props );
    this->adminService->startConsole();
}

void AdministrationActivator::stop( IBundleContext::ConstPtr context )
{
    log.log( Logger::LOG_DEBUG, "[AdministrationActivator#stop] Called." );
    this->serviceReg->unregister();
    delete (this->serviceReg);
    delete (this->adminService);
}

void AdministrationActivator::setAdministrationProvider( IAdministrationProvider* provider )
{
    log.log( Logger::LOG_DEBUG, "[AdministrationActivator#setLauncher] Called." );
    this->adminProvider = provider;
}

REGISTER_BUNDLE_ACTIVATOR_CLASS( "osgi::services::admin::AdministrationActivator", AdministrationActivator );

