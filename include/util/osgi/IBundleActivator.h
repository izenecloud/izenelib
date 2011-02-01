#ifndef IBUNDLE_ACTIVATOR_H
#define IBUNDLE_ACTIVATOR_H

#include "IBundleContext.h"

namespace izenelib{namespace osgi{


/**
 * A piece of software can be regarded as software bundle when it
 * implements the <code>BundleActivator</code> interface which is responsible
 * for starting and stopping the software bundle.<br>
 * The framework not the application developer is responsible creating,
 * starting and stopping the bundle activator instance.
 *
 * @author magr74
 */
class IBundleActivator
{
public:

    /**
     * Destroys the bundle activator object.
     */
    virtual ~IBundleActivator() {};

    /**
     * Starts the bundle activator instance and passes a bundle context
     * object which provides methods for registering services, service listeners
     * etc.
     *
     * @param context
     *     The bundle context.
     */
    virtual void start( IBundleContext::ConstPtr context ) = 0;

    /**
     * Stops the bundle activator instance.
     *
     * @param context
     *     The bundle context.
     */

    virtual void stop( IBundleContext::ConstPtr context ) = 0;
};

}}

#endif

