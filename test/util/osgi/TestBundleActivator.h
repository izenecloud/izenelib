#ifndef TEST_BUNDLE_ACTIVATOR_H
#define TEST_BUNDLE_ACTIVATOR_H

#include <util/osgi/IBundleActivator.h>
#include <util/osgi/IBundleContext.h>
#include <util/osgi/IServiceRegistration.h>
#include <util/osgi/ServiceTracker.h>

#include "IServiceBImpl.h"

using namespace izenelib::osgi;

class TestBundleActivator : public IBundleActivator, public IServiceTrackerCustomizer
{
private:
    static ServiceTracker* tracker;
    static IBundleContext* context;
    static IServiceRegistration* serviceReg;
    IServiceBImpl serviceB;

public:
    TestBundleActivator();
    virtual ~TestBundleActivator();
    virtual void start( IBundleContext::ConstPtr context );
    virtual void stop( IBundleContext::ConstPtr context );
    virtual bool addingService( const ServiceReference& ref );
    virtual void removedService( const ServiceReference& ref );
    static void unregisterServiceB();
    static void stopServiceListener();
};
#endif
