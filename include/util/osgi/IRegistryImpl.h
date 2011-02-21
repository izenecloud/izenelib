#ifndef IREGISTRY_IMPL_H
#define IREGISTRY_IMPL_H

#include <map>
#include <string>
#include <vector>

#include "BundleInfo.h"
#include "BundleInfoBase.h"
#include "ServiceInfo.h"
#include "ServiceListenerInfo.h"
#include "IServiceListener.h"
#include "ServiceEvent.h"
#include "ServiceReference.h"
#include "IRegistry.h"
#include "IService.h"
#include "Properties.h"
#include "IServiceRegistration.h"
#include "IServiceRegistrationImpl.h"

#include "util/Logger.h"
#include "util/LoggerFactory.h"
#include <util/ThreadModel.h>


namespace izenelib{namespace osgi{

using namespace izenelib::osgi::logging;
using namespace izenelib::util;
/**
 * The central class (the 'brain') of the SOF framework which
 * stores and holds all relevant data of the bundles.<br>
 * The <code>IRegistryImpl</code> class has the knowledge about
 * all registered service objects and service listeners for
 * example.<br>
 * The main tasks of the registry are:<br>* <ul>
 * <li>Storing the information about started bundles wheres
 *     the bundle data consists of bundle name, activator object
 *     registered services and registered listeners.
 * <li>Notifying the accordant service listener objects when
 *     a new service object is registered or a service
 *     object is removed.<br>
 * </ul>
 *
 * @author magr74
 */
template<class LockType=NullLock>
class IRegistryImpl : public IRegistry
{

protected:
    LockType registryLock_;
    /**
     * The logger_ instance.
     */
    static Logger &logger_;

    /**
     * The std::vector storing <code>BundleInfo</code> objects
     * which hold all bundle relevant data.
     */
    std::map<std::string,BundleInfoBase*> bundleInfoMap_;

    /**
     * This std::vector is only necessary for storing the starting order of the bundles, because
     * the <code>bundleInfoMap_</code> does not.
     */
    std::vector<std::string> bundleNames_;

    /**
     * Stores vectors of <code>ServiceInfo</code> objects
     * relating to the service name. <code>ServiceInfo</code>
     * objects store the name of the service, the service object
     * and the service properties. There can be several services
     * (several <code>ServiceInfo</code> objects) with the same
     * service name.
     *
     */
    std::map<std::string, std::vector<ServiceInfoPtr>* > serviceInfoMap_;

    /**
     * Maps <code>ServiceListenerInfo</code>  objects to the
     * service name. <code>ServiceListenerInfo</code> objects contain
     * the service name they are listen for.
     */
    std::map<std::string, std::vector<ServiceListenerInfoPtr>* > serviceListenerMap_;

    /**
     * Stops the activator object.
     *
     * @param bi
     *         The bundle information object.
     */
    virtual void stopActivator( const BundleInfoBase& bi ) ;

    /**
     * Deletes the activator object.
     *
     * @param bi
     *     The bundle information object containing all relevant
     *     bundle information.
     */
    virtual void deleteActivator( const BundleInfoBase& bi );

    /**
     * Notifies service listener objects about a specific service which was
     * registered.<br>
     * Is called when a new service listener object is registered.
     *
     * @param bundleName
     *         The name of the bundle the registered service belongs to.
     *
     * @param serviceInfoVec
     *         A std::vector of <code>ServiceInfo</code> objects which have all the same
     *         service name.
     *
     * @param serviceListenerInfoVec
     *         A std::vector of <code>ServiceListenerInfo</code> objects containing the
     *         service listener objects which are notified.
     *
     * @param serviceName
     *         The name of the service which is registered.
     */
    virtual void notifyListenersAboutRegisteredService( const std::string& bundleName, std::vector<ServiceInfoPtr>* serviceInfoVec, std::vector<ServiceListenerInfoPtr>* serviceListenerInfoVec, const std::string& serviceName );

    /**
     * Notifies service listener objects about a specific service which was
     * registered.<br>
     * Is called when a new service listener object is registered.
     *
     * @param bundleName
     *         The name of the bundle the registered service belongs to.
     *
     * @param serviceInfoVec
     *         A std::vector of <code>ServiceInfo</code> objects which have all the same
     *         service name.
     *
     * @param serviceListenerInfoVec
     *         A std::vector of <code>ServiceListenerInfo</code> objects containing the
     *         service listener objects which are notified.
     *
     * @param serviceName
     *         The name of the service which is registered.
     */
    virtual void notifyListenerAboutRegisteredService( const std::string& bundleName, std::vector<ServiceInfoPtr>* serviceInfoVec, ServiceListenerInfoPtr serviceListenerInfo, const std::string& serviceName );

    /**
     * Notifies service listener objects about a specific service which was
     * registered.<br>
     * Is called when a new service is registered.
     *
     * @param bundleName
     *         The name of the bundle the registered service belongs to.
     *
     * @param serviceInfo
     *         The <code>ServiceInfo</code> object of the service which was registered.
     *
     * @param serviceListenerInfoVec
     *         A std::vector of <code>ServiceListenerInfo</code> objects containing the
     *         service listener objects which are notified.
     *
     * @param serviceName
     *         The name of the service which is registered.
     */
    virtual void notifyListenersAboutRegisteredService( const std::string& bundleName, ServiceInfoPtr serviceInfo, std::vector<ServiceListenerInfoPtr>* serviceListenerInfoVec, const std::string& serviceName );

    /**
     * Notifies service listener objects about a specific service which is
     * deregistered.<br>
     * Is called when a service is deregistered.
     *
     * @param bundleName
     *         The name of the bundle the deregistered service belongs to.
     *
     * @param serviceInfo
     *         The <code>ServiceInfo</code> object of the service which is deregistered.
     *
     * @param serviceListenerInfoVec
     *         A std::vector of <code>ServiceListenerInfo</code> objects containing the
     *         service listener objects which must be notified.
     */
    virtual void notifyListenersAboutDeregisteredService( const std::string& bundleName, ServiceInfoPtr serviceInfo, std::vector<ServiceListenerInfoPtr>* serviceListenerInfoVec );


    /**
     * Removes a <code>ServiceInfo</code> object from the internal storage.
     *
     * @param serviceInfo
     *         The service info object which is removed.
     */
    virtual void removeFromServiceInfoVector( ServiceInfoPtr serviceInfo ) ;

    /**
     * Helper method which returns the std::vector of <code>ServiceListenerInfo</code>
     * objects.
     *
     * @param serviceName
     *         The service name the <code>ServiceListenerInfo</code> objects are mapped to.
     */
    virtual std::vector<ServiceListenerInfoPtr>* getServiceListenerInfoVector( const std::string& serviceName ) ;

    /**
     * Removes a service listener info object from the internal storage.
     *
     * @param bundleName
     *         The name of the bundle the service listener belongs to.
     *
     * @param serviceListener
     *         The service listener object.
     */
    virtual void removeFromServiceListenerInfoVector( const std::string& bundleName, ServiceListenerInfoPtr info );

    /**
     * Removes a <code>ServiceInfo</code> object from the bundle info storage.<br>
     * The <code>BundleInfo</code> object contains all relevant information of
     * a bundle (like information about registered services).
     *
     * @param bundleName
     *         The name of the bundle the deregistered service belongs to.
     *
     * @param serviceInfo
     *         The service info object which is removed.
     */
    virtual void removeDeregisteredServiceFromBundleInfo( const std::string& bundleName, ServiceInfoPtr serviceInfo ) ;

    /**
     * Returns true if the listener objects of the passed <code>ServiceListenerInfo</code> objects are equal.
     *
     * @param info1
     *         The first <code>ServiceListenerInfo</code> object.
     *
     * @param info2
     *         The second <code>ServiceListenerInfo</code> object.
     *
     * @return True, if objects are equal, otherwise false.
     */
    virtual bool areServiceListenerObjectsEqual( ServiceListenerInfoPtr info1, ServiceListenerInfoPtr info2 );

    /**
     * Sends an event to a service listener.
     *
     * @param listenerInfo
     *             The <code>ServiceListenerInfo</code> object describing the service listener which
     *             is notified.
     *
     * @param serviceInfo
     *             The <code>ServiceInfo</code> object describing the service the event is related to.
     *
     * @param eventType
     *             The service event type (e.g. REGISTER, UNREGISTER).
     */
    virtual bool callServiceListenerObject( ServiceListenerInfoPtr listenerInfo, ServiceInfoPtr, const ServiceEvent::EventType& eventType );

    /**
     * Creates an service registration object.
     *
     * @param bundleName
     *         The name of the bundle the service registration object is created for.
     * @param serviceInfo
     *         Describes the service the service registration object is created for.
     *
     * @return
     *         Returns the service registration object.
     */
    virtual IServiceRegistration::ConstPtr createServiceRegistrationObject( const std::string& bundleName, ServiceInfoPtr serviceInfo );

public:

    virtual ~IRegistryImpl();

    /**
     * Adds the service information object of an used service to the registry cache.
     *
     * @param bundleName
     *         The name of the bundle which uses the service.
     *
     * @param serviceInfo
     *         The service information object.
     */
    virtual void addUsedService( const std::string& bundleName, ServiceInfoPtr serviceInfo );

    /**
     * Removes the service information object of an used service from the registry cache.
     *
     * @param bundleName
     *         The name of the bundle which uses the service.
     *
     * @param serviceInfo
     *         The service information object.
     */
    virtual void removeUsedService( const std::string& bundleName, ServiceInfoPtr serviceInfo );


    /**
     * Adds a <code>BundleInfo</code> object to the registry.
     *
     * @param bundleInfo
     *     The <code>BundleInfo</code> object which describes a bundle.
     */
    virtual void addBundleInfo( BundleInfoBase& bundleInfo );

    /**
     * Returns the <code>BundleInfo</code> object of a specific bundle.
     *
     * @param  bundleName
     *         The name of the bundle whose bundle info object is returned.
     *
     * @return    The <code>BundleInfo</code> object.
     */
    virtual BundleInfoBase* getBundleInfo( const std::string& bundleName );

    /**
     * Returns all <code>BundleInfo</code> objects which are currently
     * stored in registry.
     *
     * @return    A std::vector of <code>BundleInfo</code> objects.
     *
     */
    virtual std::vector<BundleInfoBase*> getBundleInfos();

    /**
     * Removes the <code>BundleInfo</code> object of a specific bundle.
     *
     * @param bundleName
     *         The name of the bundle whose bundle info object is removed.
     */
    virtual void removeBundleInfo( const std::string& bundleName );

    /**
     * Removes all <code>BundleInfo</code> objects which are currently
     * stored in registry.
     */
    virtual void removeAllBundleInfos();

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
    virtual IServiceRegistration::ConstPtr addServiceInfo( const std::string& bundleName, ServiceInfoPtr serviceInfo ) ;

    /**
     * Removes a <code>ServiceInfo</code> object from the registry.
     *
     * @param bundleName
     *         The name of the bundle the service belongs to.
     *
     * @param serviceInfo
     *         The <code>ServiceInfo</code> object describing the service.
     */
    virtual void removeServiceInfo( const std::string& bundleName, ServiceInfoPtr serviceInfo ) ;

    /**
     * Returns the <code>ServiceInfo</code> object for a specific service.
     *
     * @param serviceName
     *         The name of the service.
     *
     * @return A pointer to a std::vector of pointers to <code>ServiceInfo</code> objects.
     */
    virtual std::vector<ServiceInfoPtr>* getServiceInfo( const std::string &serviceName ) ;

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
    virtual void addServiceListener( const std::string& bundleName, ServiceListenerInfoPtr serviceListenerInfo ) ;

    /**
     * Removes a service listener object from the registry.
     *
     * @param bundleName
     *         The name of the bundle the service listener belongs to.
     *
     * @param serviceListener
     *         The pointer to the service listener object.
     */
    virtual void removeServiceListener( const std::string& bundleName, ServiceListenerInfoPtr info ) ;

};

#include "IRegistryImpl.cpp"

}}
#endif

