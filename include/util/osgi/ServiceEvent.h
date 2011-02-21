#ifndef SERVICE_EVENT_H
#define SERVICE_EVENT_H

#include <string>
#include <sstream>

#include "ServiceReference.h"

namespace izenelib{namespace osgi{

/**
 * The <code>ServiceEvent</code> class describes
 * a change in the lifecycle of a service.<br>
 * Currently there are two events defined:<br>
 * <ul>
 * <li>REGISTER: Service is registered with the framework.
 * <li>UNREGISTER: Service is unregistered with the framework.
 * </ul>
 *
 * @author magr74
 */
class ServiceEvent
{
public:

    /**
     * Definition of the events.
     */
    enum EventType { REGISTER, UNREGISTER};

private:

    /**
     * The type_ of the event.
     */
    int type_;

    /**
     * The service reference_ which represents
     * the service whose lifecycle changed.
     */
    ServiceReference reference_;

public:

    /**
     * Creates instances of class <code>ServiceEvent</code>.
     *
     * @param type_
     *         The type_ of the event. Has to be of type_ integer due to compatibility to
     *         CORBA::short.
     *
     * @param reference_
     *         Describes the service.
     */
    ServiceEvent( int type, const ServiceReference& reference );

    /**
     * Returns the type_ of the event.
     *
     * @return
     *     The event type_.
     */
    int getType() const;

    /**
     * Returns the service reference_.
     *
     * @return
     *     The <code>ServiceReference</code> object.
     */
    ServiceReference getReference() const;

    /**
     * Returns a std::string representation of the <code>ServiceEvent</code> object.
     *
     * @return
     *     The std::string representation.
     */
    std::string toString() const;

};

}}
#endif
