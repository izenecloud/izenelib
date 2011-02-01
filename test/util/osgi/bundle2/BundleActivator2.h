#ifndef BUNDLE_ACTIVATOR2_H
#define BUNDLE_ACTIVATOR2_H

#include <util/osgi/IBundleActivator.h>
#include <util/osgi/IBundleContext.h>
#include <util/osgi/IServiceTrackerCustomizer.h>
#include <util/osgi/ServiceTracker.h>

#include "../IMultiplier.h"

using namespace izenelib::osgi;

class BundleActivator2 : public IBundleActivator, public IServiceTrackerCustomizer
{
private:
    ServiceTracker* tracker;
    IMultiplier* service;

public:
    BundleActivator2();
    virtual ~BundleActivator2();
    virtual void start( IBundleContext::ConstPtr context );
    virtual void stop( IBundleContext::ConstPtr context );
    virtual bool addingService( const ServiceReference& ref );
    virtual void removedService( const ServiceReference& ref );
};

#endif
