#include <util/osgi/admin/StartBundlesFromFileCmd.h>

using namespace izenelib::osgi::admin;

StartBundlesFromFileCmd::StartBundlesFromFileCmd()
{
}

std::string StartBundlesFromFileCmd::getName()
{
    return "stbfile";
}

int StartBundlesFromFileCmd::getParameterNum()
{
    return 1;
}

std::string StartBundlesFromFileCmd::getDescription()
{
    return "Starts bundles from configuration file.";
}

std::string StartBundlesFromFileCmd::execute( IAdministrationService* const adminService, std::vector<std::string> params )
{
    adminService->startBundlesFromConfigFile( params[0] );
    return "";
}
