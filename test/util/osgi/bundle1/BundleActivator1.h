#ifndef BUNDLE_ACTIVATOR1_H
#define BUNDLE_ACTIVATOR1_H

#include <util/osgi/IBundleActivator.h>
#include <util/osgi/IBundleContext.h>
#include <util/osgi/IServiceRegistration.h>

#include "../IMultiplier.h"

using namespace izenelib::osgi;

class BundleActivator1 : public IBundleActivator
{
private:
    IMultiplier* service;
    IServiceRegistration* serviceReg;

public:
    BundleActivator1();
    virtual ~BundleActivator1();
    virtual void start( IBundleContext::ConstPtr context );
    virtual void stop( IBundleContext::ConstPtr context );
};

#endif
