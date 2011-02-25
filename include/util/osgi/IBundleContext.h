#ifndef IBUNDLE_CONTEXT_H
#define IBUNDLE_CONTEXT_H

#include <string>

#include "IService.h"
#include "Properties.h"
#include "IServiceListener.h"
#include "IServiceRegistration.h"
#include "BundleConfiguration.h"

#include <boost/shared_ptr.hpp>

namespace izenelib{namespace osgi{

/**
 * When the framework starts or stops a bundle (calling <code>start</code> or
 * <code>stop</code> of the <code>BundleActivator</code>) a bundle context object
 * is passed. This <code>IBundleContext</code> object provides methods for registering
 * services, service listeners etc.<br>
 * It represents a means for the software bundle developer in order to communicate with
 * the framework.
 *
 * @author magr74
 */
class IBundleContext
{
public:

    /**
     * Type definition for a constant pointer.
     */
    typedef IBundleContext* const ConstPtr;
    typedef boost::shared_ptr<BundleConfiguration> BundleConfigurationPtr;

    /**
     * Desctructor which is called when object is deleted.
     */
    virtual ~IBundleContext() {};

    /**
     * Returns the name of the bundle.
     *
     * @return
     * The name of the bundle.
     */
    virtual std::string getBundleName() = 0;

    /**
     * Bind BundleConfiguration to Context
     * @return 
     */
    virtual void bindConfiguration(BundleConfigurationPtr bundleConfigPtr) = 0;

    /**
     * Return BundleConfiguration
     * @return 
     */
    virtual BundleConfigurationPtr getBundleConfig() = 0;
    /**
     * Registers a service with the SOF framework. Bundles which track this service
     * are notified as soon as this service is registered.
     *
     * @param className
     *         The class name of the service which is registered.
     * @param service
     *         The pointer to the service object.
     * @param dict
     *         The properties object which describes the service object.
     * @return
     *         Returns an object of type <code>IServiceRegistration</code> which provides
     *         a method for unregistering the service object.
     *
     */
    virtual IServiceRegistration* registerService( const std::string &className, IService::ConstPtr service, const Properties &dict ) = 0;

    /**
     * Adds a service listener object to the framework. The service listener is notified when the service
     * object (the service listener listens for) is registered.
     *
     * @param serviceListener
     *             The pointer to the service listener object.
     * @param serviceName
     *             The name of the service the listener listens for.
     */
    virtual void addServiceListener( IServiceListener::ConstPtr serviceListener, const std::string &serviceName ) = 0;

    /**
     * Deregisters a service listener object.
     *
     * @param serviceListener
     *         The pointer to the service listener object.
     */
    virtual void removeServiceListener( IServiceListener::ConstPtr serviceListener ) = 0;
};

}}
#endif
