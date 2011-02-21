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
 * service_ and holds all important information regarding
 * a service_ object.
 *
 * @author magr74
 */
class ServiceReference
{
protected:

    /**
     * The name of the service_.
     */
    std::string serviceName_;

    /**
     * The properties describing the service_ object.
     */
    Properties props_;

    /**
     * The constant pointer to the service_ object.
     */
    IService* service_;

    /**
     * The logger_ instance.
     */
    static Logger& logger_;

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
     *     The service_ name.
     *
     * @param properties
     *     The properties object describing the service_ object.
     *
     * @param serv
     *     The service_ object.
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
     * Sets the name of the service_.
     *
     * @param name
     *     The service_ name.
     */
    virtual void setServiceName( const std::string& name );

    /**
     * Sets the properties of the service_.
     *
     * @param props_
     *     The service_ properties.
     */
    virtual void setServiceProperties( const Properties& props_ );

    /**
     * Sets the service_ object.
     *
     * @param service_
     *         The service_ object.
     */
    virtual void setService( IService::ConstPtr service_ );

    /**
     * Returns the service_ name.
     *
     * @return
     * The name of the service_.
     */
    virtual std::string getServiceName() const;

    /**
     * Returns the properties object.
     *
     * @return
     * The properties describing the service_ object.
     */
    virtual Properties getServiceProperties() const;

    /**
     * Returns a constant pointer to the service_ object.
     *
     * @return
     * The constant pointer to the service_ object.
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

