#ifndef IBUNDLE_CONTEXT_IMPL_H
#define IBUNDLE_CONTEXT_IMPL_H

#include <string>

#include "IService.h"
#include "IBundleContext.h"
#include "Properties.h"
#include "IServiceListener.h"
#include "IServiceRegistration.h"
#include "IRegistry.h"

#include "util/Logger.h"

namespace izenelib{namespace osgi{

using namespace izenelib::osgi::logging;
/**
 * The <code>IBundleContextImpl</code> class represents the implementation
 * of the <code>IBundleContext</code> interface.
 *
 * @author magr74
 */
class IBundleContextImpl : public IBundleContext
{
protected:

    /**
     * Each bundle gets its own bundle context object. This is the
     * name of the bundle the bundle context belongs to.
     */
    std::string bundleName_;

    BundleConfigurationPtr bundleConfigPtr_;

private:

    /**
     * The registry_ which stores all relevant information
     * about the bundles (registered services, registered listeners etc.)
     */
    IRegistry& registry_;

    /**
     * The logger_ instance.
     */
    static Logger& logger_;

public:

    /**
     * Creates instances of class <code>IBundleContextImpl</code>.
     *
     * @param bundleName_
     *         The name of the bundle the context object belongs to.
     *
     * @param reg
     *         The registry_ which stores bundle information of all
     *         bundles.
     */
    IBundleContextImpl( const std::string& bundleName_, IRegistry& reg );

    /**
     * The destructor of the bundle context.
     */
    virtual ~IBundleContextImpl();

    /**
     * Returns the name of the bundle.
     *
     * @return
     * The name of the bundle.
     */
    virtual std::string getBundleName();

    /**
     * Bind BundleConfiguration to Context
     * @return 
     */
    virtual void bindConfiguration(BundleConfigurationPtr bundleConfigPtr)
    {
        bundleConfigPtr_ = bundleConfigPtr;
    }

    /**
     * Return BundleConfiguration
     * @return 
     */
    virtual BundleConfigurationPtr getBundleConfig()
    {
        return bundleConfigPtr_;
    }

    /**
     * Registers a service.
     *
     * @see izenelib::osgi::IBundleContext::registerService
     */
    virtual IServiceRegistration* registerService( const std::string &className, IService::ConstPtr service, const Properties &dict );

    /**
     * Registers a service listener object.
     *
     * @see izenelib::osgi::IBundleContext::addServiceListener
     */
    virtual void addServiceListener( IServiceListener::ConstPtr serviceListener, const std::string &serviceName );

    /**
     * Removes a service listener object.
     *
     * @see izenelib::osgi::IBundleContext::removeServiceListener
     */
    virtual void removeServiceListener( IServiceListener::ConstPtr serviceListener );
};

}}
#endif
