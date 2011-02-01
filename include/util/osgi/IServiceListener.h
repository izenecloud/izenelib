#ifndef ISERVICE_LISTENER_H
#define ISERVICE_LISTENER_H

#include "ServiceEvent.h"

namespace izenelib{namespace osgi{
/**
 * The <code>IServiceListener</code> class defines the interface
 * for a listener object which is notified about service events
 * (e.g. service was registered or deregistered).
 *
 * @author magr74
 */
class IServiceListener
{
public:

    /**
     * Defines a constant pointer to a service listener object.
     */
    typedef IServiceListener* const ConstPtr;

    /**
     * Destroys the object.
     */
    virtual ~IServiceListener() {};

    /**
     * Is called when a service event ocurred.
     *
     * @param serviceEvent
     *         The service event.
     *
     * @return Indicates whether the service listener is interested in the
     *     service event. If the service event is of type REGISTER and the
     *     return value is set to 'true', means that the service listener wants
     *     to use the registered service. If the service event is of type UNREGISTER
     *     and the return value is set to 'true', means that the service listener don't
     *     want to use the service object any longer.
     */
    virtual bool serviceChanged( const ServiceEvent &serviceEvent ) = 0;
};

}}
#endif

