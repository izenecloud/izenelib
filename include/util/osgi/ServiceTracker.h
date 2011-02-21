#ifndef SERVICE_TRACKER_H
#define SERVICE_TRACKER_H

#include <string>

#include "IBundleContext.h"
#include "IServiceTrackerCustomizer.h"
#include "IServiceListener.h"
#include "ServiceEvent.h"

#include "util/Logger.h"
#include "util/LoggerFactory.h"


namespace izenelib{namespace osgi{

using namespace izenelib::osgi::logging;

/**
 * The <code>ServiceTracker</code> class implements the
 * <code>IServiceListener</code> interface and listens for
 * registered and unregistered services.<br>
 * The service tracker class simplifies the work of application
 * developers for tracking services.
 *
 * @author magr74
 */
class ServiceTracker : public IServiceListener
{
private:

    /**
     * Indicates that service tracking is active.
     */
    bool isTrackingActive_;

    /**
     * The bundle context.
     */
    IBundleContext::ConstPtr bundleCtxt_;

    /**
     * The customized service tracker which is notified
     * when a service is registered or deregistered.
     */
    IServiceTrackerCustomizer::ConstPtr serviceTracker_;

    /**
     * The name of the service the service tracker listens for.
     */
    std::string serviceName_;

    /**
     * The logger_ instance.
     */
    static Logger& logger_;

public:

    /**
     * Creates instances of class <code>ServiceTracker</code>.
     *
     * @param bc
     *     The bundle context.
     *
     * @param servName
     *     The name of the service the service tracker listens for.
     *
     * @param customizer
     *     The service tracker customizer which is notified when
     *     a service is registered or deregistered.
     */
    ServiceTracker( IBundleContext::ConstPtr bc, const std::string &servName,
                    IServiceTrackerCustomizer::ConstPtr customizer );

    /**
     * Destroys the service tracker object.
     */
    ~ServiceTracker();

    /**
     * Starts tracking for the specified service name. From now on
     * the <code>IServiceTrackerCustomizer</code> instance is notified
     * if a service is registered or deregistered.
     */
    void startTracking();

    /**
     * Stops service tracking. From now on
     * the <code>IServiceTrackerCustomizer</code> instance
     * is no longer notified
     * if a service is registered or deregistered.
     */
    void stopTracking();

    /**
     * Is called by the framework if the lifecycle of a service
     * changed (registering, deregistering).
     *
     * @param serviceEvent
     *     The event which occurred (registering, deregistering).
     *
     * @return True, if the service tracker is interested in the
     *     changed service, otherwise false.
     */
    bool serviceChanged( const ServiceEvent &serviceEvent );

};

}}

#endif

