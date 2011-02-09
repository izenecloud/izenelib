#include <iostream>
#include <vector>
#include <boost/test/unit_test.hpp>

#include <util/osgi/Launcher.h>
#include <util/osgi/BundleInfoBase.h>
#include <util/osgi/IRegistry.h>
#include <util/osgi/BundleConfiguration.h>
#include <util/osgi/util/Logger.h>
#include <util/osgi/util/LoggerFactory.h>
#include <util/osgi/LibraryCreator.h>
#include <util/osgi/util/SingleThreaded.h>

#include "TestBundleActivator.h"
#include "TestHelper.h"

using namespace std;

using namespace izenelib::osgi;
using namespace izenelib::osgi::logging;

BOOST_AUTO_TEST_SUITE(Laucher_test)
/**
 * Tests whether it is possible to load bundles locally and from library.
 */
/*
BOOST_AUTO_TEST_CASE(load_test)
{
    BundleConfiguration bundleConf1( "bundle1", "BundleActivator2", ".", "libbundle2.so" );
    BundleConfiguration bundleConf2( "bundle2", "TestBundleActivator" );
    vector<BundleConfiguration> bundleConfVec;
    bundleConfVec.push_back( bundleConf1 );
    bundleConfVec.push_back( bundleConf2 );

    Launcher<SingleThreaded,LibraryCreator> launcher;
    launcher.start( bundleConfVec );
}
*/
/**
 * Tests whether service listeners can be registered.
 */
BOOST_AUTO_TEST_CASE(register_test)
{
    BundleConfiguration bundleConf1( "bundle1", "BundleActivator1", ".", "libbundle1.so" );
    BundleConfiguration bundleConf2( "bundle2", "TestBundleActivator" );
    vector<BundleConfiguration> bundleConfVec;
    bundleConfVec.push_back( bundleConf1 );
    bundleConfVec.push_back( bundleConf2 );

    Launcher<SingleThreaded,LibraryCreator> launcher;
    launcher.start( bundleConfVec );

    IRegistry& registry = launcher.getRegistry();

    BOOST_CHECK(TestHelper::isServiceRegisteredByBundle( registry, "bundle1", "IMultiplier", 1 ) == 1);

    BOOST_CHECK(TestHelper::isServiceListenerRegisteredByBundle( registry, "bundle2", "ServiceA" ) == 1);
}

#if 0

/**
 * Tests whether services can be used.
 */
BOOST_AUTO_TEST_CASE(usedservice_test)
{
    // Registers service listener for 'ServiceB'
    // Registers 'ServiceA' with properties 'instance=1'
    // Registers 'ServiceA' with properties 'instance=2'
    BundleConfiguration bundleConf1( "bundle1", "BundleActivator2", ".", "libbundle2.so" );

    // Registers service listener for 'ServiceA'
    // Registers 'ServiceB' with properties 'instance=1'
    BundleConfiguration bundleConf2( "bundle2", "TestBundleActivator" );

    vector<BundleConfiguration> bundleConfVec;
    bundleConfVec.push_back( bundleConf1 );
    bundleConfVec.push_back( bundleConf2 );

    Launcher<SingleThreaded,LibraryCreator>  launcher;
    launcher.start( bundleConfVec );

    IRegistry& registry = launcher.getRegistry();

    BOOST_CHECK(TestHelper::isServiceUsedByBundle( registry, "bundle1", "ServiceB", 1 ) == 1);

    // The bundle 'bundle1' registers two services of name 'ServiceA' but
    // 'bundle2' only uses one of them.
    BOOST_CHECK(TestHelper::isServiceUsedByBundle( registry, "bundle2", "ServiceA", 1 ) == 1);
}

/**
 * Tests whether services can be registered.
 */
BOOST_AUTO_TEST_CASE(registerservice_test)
{
    BundleConfiguration bundleConf1( "bundle1", "BundleActivator2", ".", "libbundle2.so" );
    BundleConfiguration bundleConf2( "bundle2", "TestBundleActivator" );
    vector<BundleConfiguration> bundleConfVec;
    bundleConfVec.push_back( bundleConf1 );
    bundleConfVec.push_back( bundleConf2 );

    Launcher<SingleThreaded,LibraryCreator>  launcher;
    launcher.start( bundleConfVec );

    IRegistry& registry = launcher.getRegistry();

    BOOST_CHECK( TestHelper::isServiceRegisteredByBundle( registry, "bundle1", "ServiceA", 1 ) == 2);

    BOOST_CHECK( TestHelper::isServiceRegisteredByBundle( registry, "bundle2", "ServiceB", 1 ) == 1);
}

/**
 * Tests whether services can be deregistered.
 */
BOOST_AUTO_TEST_CASE(unregisterservice_test)
{
    BundleConfiguration bundleConf1( "bundle1", "BundleActivator2", ".", "libbundle2.so" );
    BundleConfiguration bundleConf2( "bundle2", "TestBundleActivator" );
    vector<BundleConfiguration> bundleConfVec;
    bundleConfVec.push_back( bundleConf1 );
    bundleConfVec.push_back( bundleConf2 );

    Launcher<SingleThreaded,LibraryCreator>  launcher;
    launcher.start( bundleConfVec );

    TestBundleActivator::unregisterServiceB();

    IRegistry& registry = launcher.getRegistry();
    BOOST_CHECK( TestHelper::isServiceRegisteredByBundle( registry, "bundle2", "ServiceB", 1 ) == 0);

    BOOST_CHECK( TestHelper::isServiceUsedByBundle( registry, "bundle1", "ServiceB", 1 ) == 0);

    BOOST_CHECK( TestHelper::isServiceRegisteredByBundle( registry, "bundle1", "ServiceA", 1 ) == 2);

    BOOST_CHECK( TestHelper::isServiceListenerRegisteredByBundle( registry, "bundle1", "ServiceB" ) == 1);

    BOOST_CHECK( TestHelper::isServiceListenerRegisteredByBundle( registry, "bundle2", "ServiceA" ) == 1);
}

/**
 * Tests whether service listener can be deregistered.
 */
BOOST_AUTO_TEST_CASE(unregisterlistener_test)
{
    // Registers service listener for 'ServiceB'
    // Registers 'ServiceA' with properties 'instance=1'
    // Registers 'ServiceA' with properties 'instance=2'
    BundleConfiguration bundleConf1( "bundle1", "BundleActivator2", ".", "libbundle2.so" );

    // Registers service listener for 'ServiceA'
    // Registers 'ServiceB' with properties 'instance=1'
    BundleConfiguration bundleConf2( "bundle2", "TestBundleActivator" );

    vector<BundleConfiguration> bundleConfVec;
    bundleConfVec.push_back( bundleConf1 );
    bundleConfVec.push_back( bundleConf2 );

    Launcher<SingleThreaded,LibraryCreator>  launcher;
    launcher.start( bundleConfVec );

    IRegistry& registry = launcher.getRegistry();

    BOOST_CHECK( TestHelper::isServiceListenerRegisteredByBundle( registry, "bundle2", "ServiceA" ) == 1);

    // Stops the service tracker
    TestBundleActivator::stopServiceListener();

    BOOST_CHECK( TestHelper::isServiceListenerRegisteredByBundle( registry, "bundle2", "ServiceA" ) == 0);
}

/**
 * Tests whether a bundle can be stopped.
 */
BOOST_AUTO_TEST_CASE(stop_test)
{

    // Registers service listener for 'ServiceB'
    // Registers 'ServiceA' with properties 'instance=1'
    // Registers 'ServiceA' with properties 'instance=2'
    BundleConfiguration bundleConf1( "bundle1", "BundleActivator2", ".", "libbundle2.so" );

    // Registers service listener for 'ServiceA'
    // Registers 'ServiceB' with properties 'instance=1'
    BundleConfiguration bundleConf2( "bundle2", "TestBundleActivator" );

    vector<BundleConfiguration> bundleConfVec;
    bundleConfVec.push_back( bundleConf1 );
    bundleConfVec.push_back( bundleConf2 );

    Launcher<SingleThreaded,LibraryCreator>  launcher;
    launcher.start( bundleConfVec );

    IRegistry& registry = launcher.getRegistry();

    launcher.stopBundle( "bundle2" );

    BOOST_CHECK( TestHelper::isBundleStarted( registry, "bundle2" ) == false );

    BOOST_CHECK( TestHelper::isServiceUsedByBundle( registry, "bundle1", "ServiceB", 1 ) == 0 );
}

/**
 * Tests whether all bundles can be stopped.
 */
BOOST_AUTO_TEST_CASE(stopall_test)
{
    BundleConfiguration bundleConf1( "bundle1", "BundleActivator2", ".", "libbundle2.so" );
    BundleConfiguration bundleConf2( "bundle2", "TestBundleActivator" );
    vector<BundleConfiguration> bundleConfVec;
    bundleConfVec.push_back( bundleConf1 );
    bundleConfVec.push_back( bundleConf2 );

    Launcher<SingleThreaded,LibraryCreator>  launcher;
    launcher.start( bundleConfVec );

    IRegistry& registry = launcher.getRegistry();

    launcher.stop();

    BOOST_CHECK( TestHelper::isBundleStarted( registry, "bundle2" ) == false);

    BOOST_CHECK( TestHelper::isBundleStarted( registry, "bundle1" ) == false);
}
#endif
BOOST_AUTO_TEST_SUITE_END()

