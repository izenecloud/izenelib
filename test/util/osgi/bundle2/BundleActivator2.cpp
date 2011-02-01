#include "BundleActivator2.h"

#include <iostream>

#include <util/osgi/ObjectCreator.h>

#include "../IMultiplier.h"

using namespace std;
using namespace izenelib::osgi;

BundleActivator2::BundleActivator2()
{
    this->tracker = 0;
    this->service = 0;
}

BundleActivator2::~BundleActivator2()
{
    // Deallocate memory
}

void BundleActivator2::start(IBundleContext::ConstPtr context)
{
    this->tracker = new ServiceTracker( context, "IMultiplier", this );
    this->tracker->startTracking();
}

void BundleActivator2::stop(IBundleContext::ConstPtr context)
{
    this->tracker->stopTracking();
    delete ( this->tracker );
}

bool BundleActivator2::addingService( const ServiceReference& ref )
{
    if ( ref.getServiceName() == "IMultiplier" )
    {
        Properties props = ref.getServiceProperties();
        if ( props.get( "instance" ) == "1" )
        {
            this->service = static_cast<IMultiplier*> ( ref.getService() );
            cout << "[BundleActivator2#addingService] Calling IMultiplier..." << endl;
            int value = this->service->multiply( 47, 11 );
            cout << "[BundleActivator2#addingService] Returned value of IMultiplier: " << value << endl;
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

void BundleActivator2::removedService( const ServiceReference& ref )
{
}

REGISTER_BUNDLE_ACTIVATOR_CLASS( "BundleActivator2", BundleActivator2 )

