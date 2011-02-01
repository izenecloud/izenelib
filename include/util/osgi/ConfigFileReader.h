#ifndef CONFIG_FILE_READER_H
#define CONFIG_FILE_READER_H

#include <string>
#include <vector>

#include "util/Logger.h"
#include "util/LoggerFactory.h"
#include "BundleConfiguration.h"

namespace izenelib{namespace osgi{

using namespace izenelib::osgi::logging;

/**
 * The <code>ConfigFileReader</code> class is responsible for reading a configuration file
 * which contains bundle configuration items. Each line in the configuration file
 * represents a bundle configuration, whereas the bundle configuration consists of following
 * data (separated by comma): <br>
 * <ul>
 * <li>bundle name
 * <li>activator class name
 * <li>library path (optional)
 * <li>library name (optional)<br>
 * </ul>
 * Example for configuration file:<br>
 * test_bundle1,TestBundleActivator1,c:/temp/libraries,test.dll<br>
 * test_bundle2,TestBundleActivator2<br>
 * ...
 *
 * @author magr74
 */
class ConfigFileReader
{
private:

    /**
     * Creates instances of class <code>ConfigFileReader</code>. The constructor
     * is declared as private to avoid that instances can be created. Reason:
     * The class does only contain static methods.
     */
    ConfigFileReader();

    /**
     * The logger instance.
     */
    static Logger& logger;

public:

    /**
     * Reads bundle configurations from file.
     *
     * @param filename
     *         The name of the configuration file.
     * @return
     *         A std::vector of <code>BundleConfiguration</code> objects.
     *
     * @exception    ConfigurationException
     *             Is thrown if configuration file can not be read.
     */
    static std::vector<BundleConfiguration> readFromFile( const std::string& filename );
};

}}
#endif
