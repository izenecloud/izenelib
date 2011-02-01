#ifndef STOP_BUNDLE_CMD_H
#define STOP_BUNDLE_CMD_H

#include <string>
#include <vector>

#include "ConsoleCommand.h"


namespace izenelib{namespace osgi{namespace admin{
/**
 * The <code>StopBundleCmd<code> represents a console command
 * which stops a bundle.<br>
 * Example:<br>
 * <code>spb <bundle_name></code>
 *
 * @author magr74
 */
class StopBundleCmd : public ConsoleCommand
{
public:

    /**
     * Creates instances of class </code>StopBundleCmd</code>.
     */
    StopBundleCmd();

    /**
     * @see sof::services::admin::ConsoleCommand::getName
     */
    std::string getName();

    /**
     * @see sof::services::admin::ConsoleCommand::getDescription
     */
    std::string getDescription();

    /**
     * @see sof::services::admin::ConsoleCommand::getParameterNum
     */
    int getParameterNum();

    /**
     * @see sof::services::admin::ConsoleCommand::execute
     */
    std::string execute( IAdministrationService* const adminService, std::vector<std::string> params );
};

}}}

#endif

