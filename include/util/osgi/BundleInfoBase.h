#ifndef BUNDLE_INFO_BASE_H
#define BUNDLE_INFO_BASE_H

#include <vector>
#include <string>

#include "IBundleActivator.h"
#include "IBundleContext.h"
#include "ServiceInfo.h"
#include "ServiceListenerInfo.h"

#include "util/LoggerFactory.h"
#include "util/Logger.h"

namespace izenelib{namespace osgi{

using namespace izenelib::osgi::logging;

/**
 * The <code>BundleInfoBase</code> class contains all the relevant information
 * of a specific bundle:
 * <ul>
 * <li>The name of the bundle.
 * <li>The bundle context.
 * <li>The list of services which were registered by the bundle.
 * <li>The list of services which are used by the bundle.
 * <li>The list of service listeners.<br>
 * </ul>
 *
 * @author magr74
 */
class BundleInfoBase
{
private:

    /**
     * The name of the bundle.
     */
    std::string bundleName_;

    /**
     * The bundle context.
     */
    IBundleContext::ConstPtr bundleContext_;

    /**
     * Contains <code>ServiceInfo</code> objects of all registered services.
     */
    std::vector<ServiceInfoPtr> registeredServices_;

    /**
     * Contains <code>ServiceInfo</code> objects of all used services.
     */
    std::vector<ServiceInfoPtr> usedServices_;

    /**
     * Contains the registered service listeners.
     */
    std::vector<ServiceListenerInfoPtr> registeredListeners_;

    /**
     * Indicates if bundle is a SOF bundle or user bundle.
     */
    bool isFwBundle_;

protected:

    /**
     * The logger_ instance.
     */
    static Logger& logger_;

public:

    /**
     * Creates instances of class <code>BundleInfo</code>.
     *
     * @param bundleName_
     *     The name of the bundle.
     * @param isFwBundle_
     *     Indicates if bundle is a SOF or user bundle.
     * @param bundleCtxt
     *     The bundle context.
     */
    BundleInfoBase( const std::string& bundleName_, bool isFwBundle_, IBundleContext::ConstPtr bundleCtxt );

    /**
     * The destructor for cleaning resources.
     */
    virtual ~BundleInfoBase();

    /**
     * Returns the pointer to the bundle context object. The bundle context
     * provides functionality for registering services, adding service listeners etc.
     *
     * @return
     * The bundle context object.
     */
    virtual IBundleContext::ConstPtr getBundleContext();

    /**
     * Adds information about a service which was registered by the bundle.
     *
     * @param serviceInfo
     *         The service information object.
     */
    virtual void addRegisteredService( ServiceInfoPtr serviceInfo );

    /**
     * Removes information about the specified service.
     *
     * @param serviceInfo
     *         The service information object.
     */
    virtual void removeDeregisteredService( const ServiceInfoPtr serviceInfo );

    /**
     * Adds information about the specified service which is used by the
     * bundle.
     *
     * @param serviceInfo
     *         The service information object.
     */
    virtual void addUsedService( ServiceInfoPtr serviceInfo );

    /**
     * Removes the information about the specified service which was used by the
     * bundle.
     *
     * @param serviceInfo
     *         The service information object.
     */
    virtual void removeUsedService( ServiceInfoPtr serviceInfo );

    /**
     * Removes the information about the specified service which was used by the
     * bundle.
     *
     * @param serviceName
     *         The name of the service.
     */
    virtual void removeUsedService( const std::string& serviceName );

    /**
     * Removes the service information about all used services.
     */
    virtual void removeAllUsedServices();

    /**
     * Adds the information about a registered service listener.
     *
     * @param listenerInfo
     *         The service listener information object.
     */
    virtual void addRegisteredListener( ServiceListenerInfoPtr listenerInfo );

    /**
     * Removes the information about a registered service listener.
     *
     * @param listenerInfo
     *         The service listener information object.
     */
    virtual void removeRegisteredListener( ServiceListenerInfoPtr listenerInfo );

    /**
     * Returns the name of the bundle.
     *
     * @return
     *     The name of the bundle.
     */
    virtual std::string getBundleName() const;

    /**
     * Returns a std::vector which contains all service information objects
     * of registered services.
     *
     * @return
     * Vector of <code>ServiceInfo</code> instances.
     */
    virtual std::vector<ServiceInfoPtr> getRegisteredServices() const;

    /**
     * Returns a std::vector which contains all service information objects
     * of used services.
     *
     * @return
     * Vector of <code>ServiceInfo</code> instances.
     */
    virtual std::vector<ServiceInfoPtr> getUsedServices() const;

    /**
     * Returns a std::vector which contains all service information objects
     * of registered service listeners.
     *
     * @return
     * Vector of <code>ServiceListenerInfo</code> instances.
     */
    virtual std::vector<ServiceListenerInfoPtr> getRegisteredListeners() const;

    /**
     * Returns true if bundle is a framework bundle (started by SOF itself).
     *
     * @return
     *     True, if bundle is a framework bundle otherwise false.
     */
    virtual bool isFrameworkBundle() const;

    /**
     * Returns the std::string representation of all information stored
     * in the <code>BundleInfo</code> object.
     *
     * @return
     * The std::string representation of the bundle info object.
     */
    virtual std::string toString() const;
};

}}
#endif

