#ifndef SERVICE_INFO_H
#define SERVICE_INFO_H

#include <string>
#include <sstream>

#include "IService.h"
#include "Properties.h"

#include "util/LoggerFactory.h"
#include "util/Logger.h"
#include "util/SmartPtr.h"

namespace izenelib{namespace osgi{

using namespace izenelib::osgi::logging;

/**
 * The <code>ServiceInfo</code> object is a helper class
 * for internal use of the framework.<br>
 * A <code>ServiceInfo</code> object holds all important
 * information about a service object.
 *
 * @author magr74
 */
class ServiceInfo
{
protected:

    /**
     * Defines a constant pointer to the service object.
     */
    IService* service;

    /**
     * The name of the service object.
     */
    std::string serviceName;

    /**
     * Holds the properties of a service object.
     */
    Properties props;

    /**
     * The logger instance.
     */
    static Logger& logger;

public:

    virtual bool equals( const ServiceInfo& info1, const ServiceInfo& info2 );

    /**
     * Creates instances of class <code>ServiceInfo</code>.
     *
     * @param servName
     *         The service name.
     *
     * @param service
     *         The constant pointer to the service object.
     *
     * @param properties
     *         The properties describing the service object.
     */
    ServiceInfo( const std::string &servName, IService::ConstPtr service, const Properties &properties );

    /**
     * Copys instances of class <code>ServiceInfo</code>.
     *
     * @param serviceInfo
     *         The service info object which is copied.
     */
    ServiceInfo( const ServiceInfo &serviceInfo );

    /**
     * Destroys the <code>ServiceInfo</code> object.
     */
    virtual ~ServiceInfo();

    /**
     * Returns the service name.
     *
     * @return
     * The name of the service.
     */
    virtual std::string getServiceName() const;

    /**
     * Returns the properties of the service.
     *
     * @return
     *     The properties object.
     */
    virtual Properties getProperties() const;

    /**
     * Returns a constant pointer to the service object.
     *
     * @return
     *     The service object.
     */
    virtual IService::ConstPtr getService() const;

    /**
     * Returns a std::string representation of the
     * <code>ServiceInfo</code> object.
     *
     * @return
     * A std::string containing all data of the
     * <code>ServiceInfo</code> object.
     */
    virtual std::string toString() const;

    /**
     * Assigns the passed <code>ServiceInfo</code> object to this object.
     *
     * @param serviceInfo
     *         The service info object which is assigned to this.
     *
     * @return
     *         This service info object.
     */
    virtual ServiceInfo& operator=( const ServiceInfo &serviceInfo );

    /**
     * Compares two <code>ServiceInfo</code> objects.
     *
     * @return
     * True, if the passed service info object
     * is equal to this object, otherwise false.
     */
    virtual bool operator==( const ServiceInfo& serviceInfo1 );
};

/**
 * Smart pointer for <code>ServiceInfo</code> objects.
 */
typedef SmartPtr<ServiceInfo> ServiceInfoPtr;

}}
#endif

