#ifndef DUMP_ALL_BUNDLES_CMD_H
#define DUMP_ALL_BUNDLES_CMD_H

#include <string>
#include <vector>

#include "ConsoleCommand.h"

namespace izenelib{namespace osgi{namespace admin{
/**
 * The <code>DumpAllBundlesCmd<code> represents a console command
 * which dumps the name of all started bundles<br>
 * Example:<br>
 * <code>dab <bundle_name></code>
 *
 * @author magr74
 */
class DumpAllBundlesCmd : public ConsoleCommand
{
public:

    /**
     * Creates instances of class </code>DumpAllBundlesCmd</code>.
     */
    DumpAllBundlesCmd();

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

