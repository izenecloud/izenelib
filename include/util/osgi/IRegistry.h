#ifndef IREGISTRY_H
#define IREGISTRY_H

#include <string>
#include <vector>

#include "BundleInfoBase.h"
#include "ServiceInfo.h"
#include "ServiceListenerInfo.h"
#include "IService.h"
#include "Properties.h"
#include "IServiceRegistration.h"


namespace izenelib{namespace osgi{

/**
 * The <code>IRegistry</code> class defines the interface of the
 * registry implementation (see class {@link izenelib::osgi::IRegistryImpl}), which holds
 * all relevant bundle data.<br>
 * This interface was introduced in order to avoid template dependency for classes which use
 * the registry. Other classes use the registry only via the interface.
 *
 * @author magr74
 */
class IRegistry
{
public:
    virtual ~IRegistry(){}

    /**
     * Adds a <code>BundleInfo</code> object to the registry.
     *
     * @param bundleInfo
     *     The <code>BundleInfo</code> object which describes a bundle.
     */
    virtual void addBundleInfo( BundleInfoBase& bundleInfo ) = 0;

    /**
     * Returns the <code>BundleInfo</code> object of a specific bundle.
     *
     * @param  bundleName
     *         The name of the bundle whose bundle info object is returned.
     *
     * @return
     *         The <code>BundleInfo</code> object.
     */
    virtual BundleInfoBase* getBundleInfo( const std::string& bundleName ) = 0;

    /**
     * Returns all <code>BundleInfo</code> objects which are currently
     * stored in registry.
     *
     * @return
     *         A std::vector of <code>BundleInfo</code> objects.
     *
     */
    virtual std::vector<BundleInfoBase*> getBundleInfos() = 0;

    /**
     * Removes the <code>BundleInfo</code> object of a specific bundle.
     *
     * @param bundleName
     *         The name of the bundle whose bundle info object is removed.
     */
    virtual void removeBundleInfo( const std::string& bundleName ) = 0;

    /**
     * Removes all <code>BundleInfo</code> objects which are currently
     * stored in registry.
     */
    virtual void removeAllBundleInfos() = 0;

    /**
     * Adds a <code>ServiceInfo</code> object to the registry.
     *
     * @param bundleName
     *         The name of the bundle the service belongs to.
     *
     * @param serviceName
     *         The name of the service.
     *
     * @param serviceInfo
     *         The <code>ServiceInfo</code> object describing the service.
     */
    virtual IServiceRegistration::ConstPtr addServiceInfo( const std::string& bundleName, ServiceInfoPtr serviceInfo ) = 0;

    /**
     * Removes a <code>ServiceInfo</code> object from the registry.
     *
     * @param bundleName
     *         The name of the bundle the service belongs to.
     *
     * @param serviceInfo
     *         The <code>ServiceInfo</code> object describing the service.
     */
    virtual void removeServiceInfo( const std::string& bundleName, ServiceInfoPtr serviceInfo ) = 0;

    /**
     * Returns the <code>ServiceInfo</code> object for a specific service.
     *
     * @param serviceName
     *         The name of the service.
     *
     * @return
     *         A pointer to a std::vector of pointers to <code>ServiceInfo</code> objects.
     */
    virtual std::vector<ServiceInfoPtr>* getServiceInfo( const std::string &serviceName ) = 0;

    /**
     * Adds the service information object of an used service to the registry cache.
     *
     * @param bundleName
     *         The name of the bundle which uses the service.
     *
     * @param serviceInfo
     *         The service information object.
     */
    virtual void addUsedService( const std::string& bundleName, ServiceInfoPtr serviceInfo ) = 0;

    /**
     * Removes the service information object of an used service from the registry cache.
     *
     * @param bundleName
     *         The name of the bundle which uses the service.
     *
     * @param serviceInfo
     *         The service information object.
     */
    virtual void removeUsedService( const std::string& bundleName, ServiceInfoPtr serviceInfo ) = 0;

    /**
     * Adds a service listener object to the registry.
     *
     * @param bundleName
     *         The name of the bundle the service listener belongs to.
     *
     * @param serviceListener
     *         The pointer to the service listener object.
     *
     * @param serviceName
     *         The name of the service.
     */
    virtual void addServiceListener( const std::string& bundleName, ServiceListenerInfoPtr listenerInfo ) = 0;

    /**
     * Removes a service listener object from the registry.
     *
     * @param bundleName
     *         The name of the bundle the service listener belongs to.
     *
     * @param serviceListener
     *         The pointer to the service listener object.
     */
    virtual void removeServiceListener( const std::string& bundleName, ServiceListenerInfoPtr info ) = 0;

};

}}
#endif

