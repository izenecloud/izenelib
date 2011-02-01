#include <util/osgi/admin/StopAllBundlesCmd.h>

using namespace izenelib::osgi::admin;

StopAllBundlesCmd::StopAllBundlesCmd()
{
}

std::string StopAllBundlesCmd::getName()
{
    return "spab";
}

int StopAllBundlesCmd::getParameterNum()
{
    return 0;
}

std::string StopAllBundlesCmd::getDescription()
{
    return "Stops all bundles.";
}

std::string StopAllBundlesCmd::execute( IAdministrationService* const adminService, std::vector<std::string> params )
{
    adminService->stopAllBundles();
    return "";
}
