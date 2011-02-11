#ifndef IADMINISTRATION_SERVICE_H
#define IADMINISTRATION_SERVICE_H

#include <string>
#include <vector>

#include <util/osgi/IService.h>

namespace izenelib{namespace osgi{namespace admin{

/**
 * The <code>IAdministrationService</code> interface
 * defines methods for administrating the framework
 * (stopping and starting bundles, dumping bundle
 * information etc.).<br>
 * Each command which is executed in the administration
 * console has access to this interface.<br>
 * The administration service is not registered with
 * the framework and can not be used by other bundles.
 *
 * @author magr74
 */
class IAdministrationService : public IService
{
public:

    virtual ~IAdministrationService(){}

    /**
     * Returns a std::vector containing all names of the bundles
     * which are currently started.
     *
     * @return
     *     A std::vector of bundle names.
     */
    virtual std::vector<std::string> getBundleNames() = 0;

    /**
     * Dumps the bundle information of the specified bundle.
     *
     * @param bundleName
     *     The name of the bundle which is dumped.
     *
     * @return
     *     The std::string containing the complete bundle information
     *     (registered services and service listeners etc.)
     */
    virtual std::string dumpBundleInfo( const std::string& bundleName ) = 0;

    /**
     * Dumps the name of all started bundles.
     *
     * @return
     *     The std::string containing the names of all started
     *     bundles.
     */
    virtual std::string dumpAllBundleNames() = 0;

    /**
     * Stops the specified bundle.
     *
     * @param bundleName
     *         The name of the bundle which is stopped.
     */
    virtual void stopBundle( const std::string& bundleName ) = 0;

    /**
     * Stops all bundles.
     */
    virtual void stopAllBundles() = 0;

    /**
     * Starts a bundle which must be loaded from a DLL.
     *
     * @param bundleName
     *         The name of the bundle which is started.
     *
     * @param className
     *         The class name of the activator.
     *
     * @param libPath
     *         The library path where the DLL can be find.
     *
     * @param libName
     *         The name of the DLL which is loaded.
     */
    virtual void startBundleFromDLL( const std::string& bundleName, const std::string& className, const std::string& libPath, const std::string& libName ) = 0;

    /**
     * Starts a 'local' bundle.
     *
     * @param bundleName
     *     The name of the bundle.
     *
     * @param className
     *     The name of the activator class.
     */
    virtual void startBundle( const std::string& bundleName, const std::string& className ) = 0;

    /**
     * Starts bundles which are defined in a configuration file.
     *
     * @param configFile
     *     The configuration file which contains the bundle data
     *     (activator class name, name of the DLL etc.)
     */
    virtual void startBundlesFromConfigFile( const std::string& configFile ) = 0;
};

}}}
#endif

