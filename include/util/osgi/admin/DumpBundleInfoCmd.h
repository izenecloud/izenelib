#ifndef DUMP_BUNDLE_INFO_CMD_H
#define DUMP_BUNDLE_INFO_CMD_H

#include <string>
#include <vector>

#include "ConsoleCommand.h"

namespace izenelib{namespace osgi{namespace admin{

/**
 * The <code>DumpBundleInfoCmd<code> represents a console command
 * which dumps all bundle data of a specific bundle.<br>
 * Example:<br>
 * <code>dbi <bundle_name></code>
 *
 * @author magr74
 */
class DumpBundleInfoCmd : public ConsoleCommand
{
public:

    /**
     * Creates instances of class </code>DumpBundleInfoCmd</code>.
     */
    DumpBundleInfoCmd();

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

