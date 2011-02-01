#ifndef TEST_HELPER_H
#define TEST_HELPER_H

#include <iostream>
#include <vector>

#include <util/osgi/IRegistry.h>

using namespace std;

using namespace izenelib::osgi;

/**
 * Helper class for verifying test results.
 */
class TestHelper
{
public:

    /**
     * Checks if the specified service is registered by the given bundle.
     */
    static int isServiceRegisteredByBundle( IRegistry& reg, const string& bundleName, const string& serviceName, int propsSize );

    /**
     * Checks if the specified service is used by the given bundle.
     */
    static int isServiceUsedByBundle( IRegistry& reg, const string& bundleName, const string& serviceName, int propsSize );

    /**
     * Checks if the specified service listener is registered by the given bundle.
     */
    static int isServiceListenerRegisteredByBundle( IRegistry& reg, const string& bundleName, const string& serviceName );

    /**
     * Checks if a service is registered by the given bundle.
     */
    static int isServiceRegisteredByBundle( IRegistry& reg, const string& bundleName );

    /**
     * Checks if a service is used by the given bundle.
     */
    static int isServiceUsedByBundle( IRegistry& reg, const string& bundleName );

    /**
     * Checks if a service listener is registered by the given bundle.
     */
    static int isServiceListenerRegisteredByBundle( IRegistry& reg, const string& bundleName );

    /**
     * Checks if the specified bundle is started.
     */
    static bool isBundleStarted( IRegistry& reg, const string& bundleName );
};

#endif
