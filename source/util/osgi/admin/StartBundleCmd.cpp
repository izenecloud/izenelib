#include <util/osgi/admin/StartBundleCmd.h>

using namespace izenelib::osgi::admin;

StartBundleCmd::StartBundleCmd()
{
}

std::string StartBundleCmd::getName()
{
    return "stb";
}

int StartBundleCmd::getParameterNum()
{
    return 2;
}

std::string StartBundleCmd::getDescription()
{
    return "Starts a bundle (Parameters: bundle name, class name).";
}

std::string StartBundleCmd::execute( IAdministrationService* const adminService, std::vector<std::string> params )
{
    adminService->startBundle( params[0], params[1] );
    return "";
}
