#ifndef BUNDLE_INFO_H
#define BUNDLE_INFO_H

#include <vector>
#include <string>

#include "IBundleActivator.h"
#include "BundleInfoBase.h"

#include "util/LoggerFactory.h"
#include "util/Logger.h"

namespace izenelib{namespace osgi{

/**
 * The <code>BundleInfo</code> class contains all the relevant information
 * of a specific bundle:
 * <ul>
 * <li>The name of the bundle.
 * <li>The bundle activator.
 * <li>The bundle context.
 * <li>The list of services which were registered by the bundle.
 * <li>The list of services which are used by the bundle.
 * <li>The list of service listeners.<br>
 * </ul>
 *
 * @author magr74
 */
class BundleInfo : public BundleInfoBase
{
private:

    /**
     * The bundle activator instance.
     */
    IBundleActivator* activator_;


public:

    /**
     * Creates instances of class <code>BundleInfo</code>.
     *
     * @param bundleName
     *     The name of the bundle.
     * @param isFwBundle
     *      Indicates if bundle is a SOF or user bundle.
     * @param act
     *     The bundle activator instance.
     * @param bundleCtxt
     *     The bundle context.
     */
    BundleInfo( const string& bundleName, bool isFwBundle, IBundleActivator* act, IBundleContext::ConstPtr bundleCtxt );

    /**
     * The destructor for cleaning resources.
     */
    virtual ~BundleInfo();

    /**
     * Returns the pointer to the bundle activator instance.
     *
     * @return
     * The pointer to bundle activator instance.
     */
    virtual IBundleActivator* getBundleActivator();

};

}}
#endif
