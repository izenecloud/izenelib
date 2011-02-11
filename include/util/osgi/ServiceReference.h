#ifndef SERVICE_REFERENCE_H
#define SERVICE_REFERENCE_H

#include <string>
#include <sstream>

#include "Properties.h"
#include "IService.h"

#include "util/LoggerFactory.h"
#include "util/Logger.h"

namespace izenelib{namespace osgi{

using namespace izenelib::osgi::logging;

/**
 * The <code>ServiceReference</code> represents a
 * service and holds all important information regarding
 * a service object.
 *
 * @author magr74
 */
class ServiceReference
{
protected:

    /**
     * The name of the service.
     */
    std::string serviceName;

    /**
     * The properties describing the service object.
     */
    Properties props;

    /**
     * The constant pointer to the service object.
     */
    IService* service;

    /**
     * The logger instance.
     */
    static Logger& logger;

public:

    /**
     * Creates instances of class <code>ServiceReference</code>.
     *
     */
    ServiceReference();

    /**
     * Creates instances of class <code>ServiceReference</code>.
     *
     * @param name
     *     The service name.
     *
     * @param properties
     *     The properties object describing the service object.
     *
     * @param serv
     *     The service object.
     */
    ServiceReference( const std::string &name, const Properties &properties, const IService::ConstPtr serv );

    /**
     * Copy constructor.
     *
     * @param serviceRef
     *         The <code>ServiceReference</code> object which is copied.
     */
    ServiceReference( const ServiceReference& serviceRef );

    /**
     * The assignment operator.
     *
     * @param serviceRef
     *         The <code>ServiceReference</code> object which is assignd to this
     *         object.
     */
    virtual ServiceReference &operator=( const ServiceReference &serviceRef );

    /**
     * Destroys the <code>ServiceReference</code> object.
     */
    virtual ~ServiceReference();

    /**
     * Sets the name of the service.
     *
     * @param name
     *     The service name.
     */
    virtual void setServiceName( const std::string& name );

    /**
     * Sets the properties of the service.
     *
     * @param props
     *     The service properties.
     */
    virtual void setServiceProperties( const Properties& props );

    /**
     * Sets the service object.
     *
     * @param service
     *         The service object.
     */
    virtual void setService( IService::ConstPtr service );

    /**
     * Returns the service name.
     *
     * @return
     * The name of the service.
     */
    virtual std::string getServiceName() const;

    /**
     * Returns the properties object.
     *
     * @return
     * The properties describing the service object.
     */
    virtual Properties getServiceProperties() const;

    /**
     * Returns a constant pointer to the service object.
     *
     * @return
     * The constant pointer to the service object.
     */
    virtual IService::ConstPtr getService() const;

    /**
     * Returns the std::string representation of the <code>ServiceReference</code> object.
     *
     * @return
     * The std::string representation.
     */
    virtual std::string toString() const;

};

}}
#endif

