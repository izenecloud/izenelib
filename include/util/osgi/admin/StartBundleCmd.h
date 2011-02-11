#ifndef START_BUNDLE_CMD_H
#define START_BUNDLE_CMD_H

#include <string>
#include <vector>

#include "ConsoleCommand.h"

namespace izenelib{namespace osgi{namespace admin{

/**
 * The <code>StartBundleCmd<code> represents a console command
 * which starts a 'local' bundle.<br>
 * Example:<br>
 * <code>stb <bundle_name> <class_name></code>
 *
 * @author magr74
 */
class StartBundleCmd : public ConsoleCommand
{
public:

    /**
     * Creates instances of class </code>StartBundleCmd</code>.
     */
    StartBundleCmd();

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

