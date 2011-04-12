#include "TestBundleActivator.h"
#include <util/osgi/Properties.h>
#include <util/osgi/ObjectCreator.h>

#include <iostream>


using namespace std;
using namespace izenelib::osgi;
using namespace izenelib::osgi::logging;

IBundleContext* TestBundleActivator::context = 0;
IServiceRegistration* TestBundleActivator::serviceReg = 0;
ServiceTracker* TestBundleActivator::tracker = 0;

TestBundleActivator::TestBundleActivator()
{
    serviceReg = 0;
    tracker = 0;
}

void TestBundleActivator::start( IBundleContext::ConstPtr ctxt )
{
    context = ctxt;
    cout<<"[TestBundleActivator#start] Called."<<endl;

    tracker = new ServiceTracker( context, "ServiceA", this );
    tracker->startTracking();

    Properties props;
    props.put( "instance", "1" );

    cout<<"[TestBundleActivator#start] Register ServiceB..." <<endl;
    serviceReg = context->registerService( "ServiceB", &( this->serviceB ), props );
    cout<<"[TestBundleActivator#start] ServiceB registered." <<endl;
    cout<<"[TestBundleActivator#start] Left." <<endl;
}

TestBundleActivator::~TestBundleActivator()
{
    cout<< "[TestBundleActivator#destructor] Called." <<endl;
}

void TestBundleActivator::stop( IBundleContext::ConstPtr context )
{
    cout<< "[TestBundleActivator#stop] Called." <<endl;
    if(serviceReg)
    {
        serviceReg->unregister();
        delete serviceReg;
    }
    if(tracker)
    {
        tracker->stopTracking();
        delete tracker;
    }
}

void TestBundleActivator::unregisterServiceB()
{
    cout<< "[TestBundleActivator#unregisterServiceB] Called." <<endl;
    serviceReg->unregister();
    delete serviceReg;
    serviceReg = 0;
}

void TestBundleActivator::stopServiceListener()
{
    tracker->stopTracking();
}

bool TestBundleActivator::addingService( const ServiceReference& ref )
{
    cout<< "[TestBundleActivator#addingService] Called." <<endl;
    if ( ref.getServiceName() == "ServiceA" )
    {
        cout<< "[TestBundleActivator#addingService] ServiceA found." <<endl;
        Properties props = ref.getServiceProperties();
        if ( props.get( "instance" ) == "1" )
        {
            cout<< "[TestBundleActivator#addingService] Instance 1 found." <<endl;
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

void TestBundleActivator::removedService( const ServiceReference& ref )
{
    cout<< "[TestBundleActivator#removedService] Called." <<endl;
}


