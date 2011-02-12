#ifndef ADMINISTRATION_ACTIVATOR_H
#define ADMINISTRATION_ACTIVATOR_H

#include "IAdministrationProvider.h"
#include "IAdministrationServiceImpl.h"

#include <util/osgi/IBundleActivator.h>
#include <util/osgi/IServiceRegistration.h>
#include <util/osgi/util/LoggerFactory.h>
#include <util/osgi/util/Logger.h>

namespace izenelib{namespace osgi{namespace admin{


using namespace izenelib::osgi::logging;
using namespace izenelib::osgi::admin;

/**
 * The <code>AdministrationActivator</code> is the activator
 * class of the administration bundle which provides a administration
 * console for the user.<br>
 * This activator class is created when <code>startAdministrationBundle</code>
 * of the <code>Launcher</code> class is called.<br>
 * Note: After the <code>startAdministrationBundle</code> method is called the
 * executing thread blocks and waits for user inputs on the console.
 *
 * @author magr74
 */
class AdministrationActivator : public IBundleActivator
{
private:

    /**
     * The logger instance.
     */
    static Logger& log;

    /**
     * The administration service providing the interface for administrating
     * the OSGI runtime environment.
     */
    IAdministrationServiceImpl* adminService;

    /**
     * The administration provider the administration commands
     * are forwarded to.
     */
    IAdministrationProvider* adminProvider;

    /**
     * The <code>IServiceRegistration</code> object of
     * the administration service.
     */
    IServiceRegistration* serviceReg;


public:

    /**
     * Creates instances of class <code>AdministrationActivator</code>
     */
    AdministrationActivator();


    ~AdministrationActivator();

    /**
     * Starts the administration bundle by creating
     * a console and waits for user input.
     */
    void start( IBundleContext::ConstPtr context );

    /**
     * Stops the administration bundle.
     */
    void stop( IBundleContext::ConstPtr context );

    /**
     * Sets the <code>Launcher</code> instance to be able
     * to communicate with the framework.
     */
    void setAdministrationProvider( IAdministrationProvider* adminProvider );
};

}}}

#endif

