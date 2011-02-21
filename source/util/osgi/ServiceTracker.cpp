#include <util/osgi/ServiceTracker.h>
#include <util/osgi/IServiceTrackerCustomizer.h>
#include <util/osgi/IBundleContext.h>

using namespace izenelib::osgi;
using namespace izenelib::osgi::logging;

Logger& ServiceTracker::logger_ = LoggerFactory::getLogger( "Framework" );

ServiceTracker::ServiceTracker( IBundleContext::ConstPtr bc, const string &servName,
                                IServiceTrackerCustomizer::ConstPtr customizer ) 
    : isTrackingActive_( false ),
      bundleCtxt_( bc ), 
      serviceTracker_( customizer ),
      serviceName_( servName )
{
    logger_.log( Logger::LOG_DEBUG, "[ServiceTracker#ctor] Called, service name: %1", servName );
}

ServiceTracker::~ServiceTracker()
{
    logger_.log( Logger::LOG_DEBUG, "[ServiceTracker#destructor] Called." );
    // Bugfix for ID 3000086: ServiceTracker destructor should call stopTracking
    if ( this->isTrackingActive_ )
    {
        logger_.log( Logger::LOG_DEBUG, "[ServiceTracker#destructor] Service tracking is active, stop tracking." );
        this->stopTracking();
    }
}

void ServiceTracker::startTracking()
{
    logger_.log( Logger::LOG_DEBUG, "[ServiceTracker#startTracking] Called." );
    this->bundleCtxt_->addServiceListener( this, this->serviceName_ );
    this->isTrackingActive_ = true;
}

void ServiceTracker::stopTracking()
{
    logger_.log( Logger::LOG_DEBUG, "[ServiceTracker#stopTracking] Called." );
    this->bundleCtxt_->removeServiceListener( this );
    this->isTrackingActive_ = false;
}

bool ServiceTracker::serviceChanged( const ServiceEvent &serviceEvent )
{
    logger_.log( Logger::LOG_DEBUG, "[ServiceTracker#serviceChanged] Called, service event: %1", serviceEvent.toString() );
    bool retVal = false;
    try
    {
        if ( serviceEvent.getType() == ServiceEvent::REGISTER )
        {
            logger_.log( Logger::LOG_DEBUG, "[ServiceTracker#serviceChanged] Service is registered, service name: %1", serviceEvent.getReference().getServiceName() );
            retVal = this->serviceTracker_->addingService( serviceEvent.getReference() );
        }
        else if ( serviceEvent.getType() == ServiceEvent::UNREGISTER )
        {
            logger_.log( Logger::LOG_DEBUG, "[ServiceTracker#serviceChanged] Service is unregistered, service name: %1", serviceEvent.getReference().getServiceName() );
            this->serviceTracker_->removedService( serviceEvent.getReference() );
            retVal = true;
        }
        else
        {
            logger_.log( Logger::LOG_DEBUG, "[ServiceTracker#serviceChanged] Unhandled event, service name: %1", serviceEvent.getReference().getServiceName() );
            retVal = false;
        }
    }
    catch ( std::exception exc )
    {
        logger_.log( Logger::LOG_ERROR, "[ServiceTracker#serviceChanged] Error occurred during adding/removing service: %1", string( exc.what() ) );
    }
    // To play it safe, catch all exceptions which are not standard c++ exceptions.
    catch ( ... )
    {
        logger_.log( Logger::LOG_ERROR, "[ServiceTracker#serviceChanged] Error occurred during adding/removing service." );
    }
    return retVal;
}

