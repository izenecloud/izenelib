#ifndef BUNDLE_CONFIGURATION_H
#define BUNDLE_CONFIGURATION_H

#include <string>

namespace izenelib{namespace osgi{

/**
 * The <code>BundleConfiguration</code> describes all relevant data
 * which are necessary for the framework for starting a bundle.<br>
 *
 * @author magr74
 */
class BundleConfiguration
{
protected:
    /**
     * Represents the name of the bundle, e.g. 'test_bundle'.
     */
    std::string bundleName_;

    /**
     * Indicates the class name of the bundle activator.
     */
    std::string className_;

    /**
     * Represents the library path where the bundle is located as
     * library, e.g. as DLL on windows systems.<br>
     * Example for library path: 'c:/temp/libraries'.
     */
    std::string libraryPath_;

    /**
     * Specifies the name of the library which contains the bundle, e.g.
     * 'test_bundle.dll'
     */
    std::string libraryName_;

public:

    /**
     * Constant value which indicates that no library path is available.
     */
    static const std::string NO_LIB_PATH;

    /**
     * Constant value which indicates that no library name is available.
     */
    static const std::string NO_LIB_NAME;

    /**
     * Creates instances of class <code>BundleConfiguration</code>.
     *
     * @param bundleName_
     *         The name of the bundle.
     * @param className_
     *         The name of the bundle activator class.
     * @param libPath
     *         The path to the bundle library.
     * @param libName
     *         The name of the bundle library.
     */
    BundleConfiguration( const std::string &bundleName, const std::string &className, const std::string &libPath, const std::string &libName );

    /**
     * Creates instances of class <code>BundleConfiguration</code>.
     *
      * @param bundleName_
     *         The name of the bundle.
     * @param className_
     *         The name of the bundle activator class.
     */
    BundleConfiguration( const std::string &bundleName, const std::string &className );

    /**
     * Returns the name of the bundle.
     */
    std::string getBundleName();

    /**
     * Returns the name of the activator class.
     */
    std::string getClassName();

    /**
     * Returns the path to the bundle library.
     */
    std::string getLibraryPath();

    /**
     * Returns the name of the bundle library.
     */
    std::string getLibraryName();

    /**
     * Returns the std::string representation of the <code>BundleConfiguration</code>
     * object.
     */
    std::string toString();
};

}}

#endif

