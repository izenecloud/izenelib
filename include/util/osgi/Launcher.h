#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <vector>
#include <iostream>
#include <sstream>

#include "IBundleActivator.h"
#include "IRegistry.h"
#include "IRegistryImpl.h"
#include "BundleInfo.h"
#include "IBundleContextImpl.h"

#include "BundleInfoBase.h"
#include "admin/AdministrationActivator.h"
#include "admin/IAdministrationProvider.h"
#include "BundleConfiguration.h"
#include "ObjectCreator.h"
#include "util/Logger.h"
#include "util/LoggerFactory.h"
#include <util/ThreadModel.h>


namespace izenelib{namespace osgi{

using namespace izenelib::osgi::logging;
using namespace izenelib::osgi::admin;
using namespace izenelib::util;
/**
 * The <code>Launcher</code> class is the entry point for
 * running the SOF framework.<br>
 * The main task of this class is to provide methods
 * for starting and stopping bundles.
 *
 * @author magr74
 */
template<
class LockType = NullLock,
template <class> class CreationPolicy = NullCreator>
class Launcher : public IAdministrationProvider
{
protected:

    /**
     * The <code>ObjectCreator</code> instance which is used
     * for instantiating the <code>IBundleActivator</code>
     * objects.
     */
    ObjectCreator<IBundleActivator,CreationPolicy> objectCreator_;

    /**
     * The registry_ object which holds all relevant data of
     * all bundles. It is the central administration object.
     */
    IRegistry* registry_;

    /**
     * The logger_ instance.
     */
    static Logger& logger_;

    /**
     * Creates the registry_ instance.
     *
     * @return
     * The registry_ instance.
     */
    virtual IRegistry* createRegistry();

    /**
     * Creates the bundle context instances.
     *
     * @param bundleName
     *         The name of the bundle the bundle context object is
     *         created for.
     *
     * @return
     *         The bundle context instance.
     */
    virtual IBundleContext* createBundleContext( const std::string& bundleName );

public:

    /**
     * Creates instances of class <code>Launcher</code>.
     */
    Launcher();

    /**
     * Destroys the <code>Launcher</code> instance.
     */
    virtual ~Launcher();

    /**
     * Sets the log level of the framework. Defines
     * for example whether only error messages or
     * also debug messages shall be logged.
     *
     * @param level
     *     The log level (trace, debug, error).
     */
    virtual void setLogLevel( Logger::LogLevel level );

    /**
     * Starts bundles. The bundles which are started are
     * defined in a std::vector of <code>BundleConfiguration</code>
     * objects.
     *
     * @param configuration
     *         The std::vector of <code>BundleConfiguration</code>
     *         objects whereas each object describes what
     *         bundle shall be started.
     */
    virtual void start( std::vector<BundleConfiguration> &configuration );

    /**
     * Stops all bundles which were started.
     */
    virtual void stop();

    /**
     * Starts a specific bundle. Can be also called after
     * a <code>start()</code>.
     *
     * @param bundleConfig
     *         The object containing information which
     *         bundle must be started.
     */
    virtual void startBundle( BundleConfiguration bundleConfig );

    /**
     * Stops a bundle.
     *
     * @param bundleName
     *         The name of the bundle which is stopped.
     */
    virtual void stopBundle( const std::string& bundleName );

    /**
     * Starts the administration bundle (which
     * provides a console for user inputs).
     */
    virtual void startAdministrationBundle();

    /**
     * Returns the names of all started bundles.
     *
     * @return
     * A std::vector containing all bundle names.
     */
    virtual std::vector<std::string> getBundleNames();

    /**
     * Dumps all information (registered services,
     * registered service listeners, services in use)
     * of a bundle.
     *
     * @param bundleName
     *         The name of the bundle.
     *
     * @return
     *         A std::string containing all information
     *         about a bundle.
     */
    virtual std::string dumpBundleInfo( const std::string& bundleName );

    /**
     * Dumps the name of all started bundles.
     *
     * @return
     *     A std::string containing all bundle names.
     */
    virtual std::string dumpAllBundleNames();

    /**
     * Returns the bundle info object for the given bundle name.
     *
     * @param bundleName
     *         The bundle name.
     *
     * @return
     *         The bundle info object.
     */
    virtual BundleInfoBase& getBundleInfo( const std::string& bundleName );

    /**
     * Returns the registry_ object.
     *
     * @return
     *     The registry_ object.
     */
    virtual IRegistry& getRegistry();
};

#include "Launcher.cpp"

}}

#endif

