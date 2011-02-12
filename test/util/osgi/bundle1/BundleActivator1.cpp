#include "BundleActivator1.h"

#include <util/osgi/ObjectCreator.h>
#include <util/osgi/Properties.h>

#include "../IMultiplier.h"
#include "IMultiplierImpl.h"

using namespace izenelib::osgi;

BundleActivator1::BundleActivator1()
{
    this->service = 0;
    this->serviceReg = 0;
}


BundleActivator1::~BundleActivator1()
{
}

void BundleActivator1::start(IBundleContext::ConstPtr context)
{
    Properties props;
    props.put( "instance", "1" );

    this->service = new IMultiplierImpl();
    this->serviceReg = context->registerService( "IMultiplier", this->service, props );
}

void BundleActivator1::stop(IBundleContext::ConstPtr context)
{
    this->serviceReg->unregister();
    delete this->serviceReg;
    delete this->service;
}

REGISTER_BUNDLE_ACTIVATOR_CLASS( "BundleActivator1", BundleActivator1 )

