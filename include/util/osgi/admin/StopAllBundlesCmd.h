#ifndef STOP_ALL_BUNDLES_CMD_H
#define STOP_ALL_BUNDLES_CMD_H

#include <string>
#include <vector>

#include "ConsoleCommand.h"


namespace izenelib{namespace osgi{namespace admin{
/**
 * The <code>StopAllBundlesCmd<code> represents a console command
 * which stops all bundles.<br>
 * Example:<br>
 * <code>spab</code>
 *
 * @author magr74
 */
class StopAllBundlesCmd : public ConsoleCommand
{
public:

    /**
     * Creates instances of class </code>StopAllBundlesCmd</code>.
     */
    StopAllBundlesCmd();

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

