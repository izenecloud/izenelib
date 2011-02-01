#ifndef IADMINISTRATION_SERVICE_IMPL_H
#define IADMINISTRATION_SERVICE_IMPL_H

#include <string>
#include <vector>
#include <map>

#include "IAdministrationService.h"
#include "IAdministrationProvider.h"
#include "ConsoleCommand.h"

#include <util/osgi/util/Logger.h>
#include <util/osgi/util/LoggerFactory.h>

namespace izenelib{namespace osgi{namespace admin{

using namespace izenelib::osgi::logging;

/**
 * The <code>IAdministrationServiceImpl</code> class implements
 * the <code>IAdministrationService</code> interface which provides
 * functionality for interacting with the framework.
 */
class IAdministrationServiceImpl : public IAdministrationService
{
private:

    /**
     * The <code>Launcher</code> the calls are forwarded to.
     */
    IAdministrationProvider* adminProvider;

    /**
     * The logger instance.
     */
    static Logger& logger;

    /**
     * The std::map which contains all possible console commands.
     */
    std::map<std::string,ConsoleCommand*> cmdMap;

public:

    /**
     * Creates instances of class <code>IAdministrationServiceImpl</code>.
     *
     * @param launcher
     *         The <code>Launcher</code> object.
     */
    IAdministrationServiceImpl( IAdministrationProvider* provider );

    /**
     * Starts the administration console.
     */
    void startConsole();

    /**
     * Stops the administration console.
     */
    void stopConsole();

    /**
     * @see sof::services::admin::IAdministrationService::getBundleNames
     */
    std::vector<std::string> getBundleNames();

    /**
     * @see sof::services::admin::IAdministrationService::dumpBundleInfo
     */
    std::string dumpBundleInfo( const std::string& bundleName );

    /**
     * @see sof::services::admin::IAdministrationService::dumpAllBundleNames
     */
    std::string dumpAllBundleNames();

    /**
     * @see sof::services::admin::IAdministrationService::stopBundle
     */
    void stopBundle( const std::string& bundleName );

    /**
     * @see sof::services::admin::IAdministrationService::stopAllBundles
     */
    void stopAllBundles();

    /**
     * @see sof::services::admin::IAdministrationService::startBundleFromDLL
     */
    void startBundleFromDLL( const std::string& bundleName, const std::string& className, const std::string& libPath, const std::string& libName );

    /**
     * @see sof::services::admin::IAdministrationService::startBundle
     */
    void startBundle( const std::string& bundleName, const std::string& className );

    /**
     * @see sof::services::admin::IAdministrationService::startBundlesFromConfigFile
     */
    void startBundlesFromConfigFile( const std::string& configFile );
};

}}}
#endif

