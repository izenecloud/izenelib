#ifndef SERVICE_LISTENER_INFO_H
#define SERVICE_LISTENER_INFO_H

#include <string>

#include "util/LoggerFactory.h"
#include "util/Logger.h"
#include "util/SmartPtr.h"

#include "IServiceListener.h"

namespace izenelib{namespace osgi{

using namespace izenelib::osgi::logging;

/**
 * The <code>ServiceListenerInfo</code> class is a helper
 * class for framework internal use and holds all important
 * information about a service listener.
 *
 * @author magr74
 */
class ServiceListenerInfo
{
protected:

    /**
     * The name of the bundle the service listener belongs to.
     */
    std::string bundleName_;

    /**
     * The name of the service the service listener listens for.
     */
    std::string serviceName_;

    /**
     * The constant pointer to the service listener object.
     */
    IServiceListener* serviceListenerObj_;

    /**
     * The logger_ instance.
     */
    static Logger& logger_;

public:

    /**
     * Creates instances of class <code>ServiceListenerInfo</code>.
     *
     * @param bundleName_
     *         The name of the bundle the service listener belongs to.
     *
     * @param serviceName_
     *         The name of the service the service listener listens to.
     *
     * @param serviceListener
     *         The service listener.
     */
    ServiceListenerInfo( const std::string& bundleName_, const std::string& serviceName_, IServiceListener::ConstPtr serviceListener );

    /**
     * Copy constructor.
     *
     * @param info
     *     The <code>ServiceListenerInfo</code> object which is copied.
     */
    ServiceListenerInfo( const ServiceListenerInfo& info );

    /**
     * Destroys the object.
     */
    virtual ~ServiceListenerInfo();

    /**
     * Returns the bundle name.
     *
     * @return
     *     The name of the bundle.
     */
    virtual std::string getBundleName() const;

    /**
     * Returns the service name.
     *
     * @return
     * The name of the service.
     */
    virtual std::string getServiceName() const;

    /**
     * Returns the service listener object.
     *
     * @return
     * A constant pointer to the service listener object.
     */
    virtual IServiceListener::ConstPtr getServiceListenerObj() const;

    /**
     * Compares to instances of <code>ServiceListenerInfo</code>
     * objects.
     *
     * @return
     *     True if the <code>ServiceListenerInfo</code> objects
     *     are equal, otherwise false.
     */
    virtual bool operator==( const ServiceListenerInfo& info1 );

    /**
     * Assigns the passed <code>ServiceListenerInfo</code> object to this object.
     *
     * @param serviceListenerInfo
     *         The service listener object which is assigned to this.
     *
     * @return
     *         This service listener object.
     */
    virtual ServiceListenerInfo& operator=( const ServiceListenerInfo &serviceListenerInfo );

    /**
     * Compares two objects of type <code>ServiceListenerInfo</code>.
     *
     * @param info1
     *         The first <code>ServiceListenerInfo</code> object which is compared.
     *
     * @param info2
     *         The second <code>ServiceListenerInfo</code> object which is compared.
     *
     * @return
     *         True, if the objects are equal, otherwise false.
     */
    virtual bool equals( const ServiceListenerInfo& info1, const ServiceListenerInfo& info2 );

    /**
     * Returns a std::string representation of the
     * <code>ServiceListenerInfo</code> object.
     *
     * @return
     * A std::string containing all data of the
     * <code>ServiceListenerInfo</code>.
     */
    virtual std::string toString() const;
};

/**
 * Smart pointer for <code>ServiceInfo</code> objects.
 */
typedef SmartPtr<ServiceListenerInfo> ServiceListenerInfoPtr;

}}

#endif

