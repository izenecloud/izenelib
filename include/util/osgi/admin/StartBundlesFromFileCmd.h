#ifndef START_BUNDLES_FROM_FILE_CMD_H
#define START_BUNDLES_FROM_FILE_CMD_H

#include <string>
#include <vector>

#include "ConsoleCommand.h"


namespace izenelib{namespace osgi{namespace admin{

/**
 * The <code>StartBundlesFromFileCmd<code> represents a console command
 * which starts bundles specified in a configuration file.<br>
 * Example:<br>
 * <code>stbfile <config_file></code>
 *
 * @author magr74
 */
class StartBundlesFromFileCmd : public ConsoleCommand
{
public:

    /**
     * Creates instances of class </code>StartBundlesFromFileCmd</code>.
     */
    StartBundlesFromFileCmd();

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

