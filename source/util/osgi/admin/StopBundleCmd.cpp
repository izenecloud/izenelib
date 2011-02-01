#include <util/osgi/admin/StopBundleCmd.h>

using namespace izenelib::osgi::admin;

StopBundleCmd::StopBundleCmd()
{
}

std::string StopBundleCmd::getName()
{
    return "spb";
}

int StopBundleCmd::getParameterNum()
{
    return 1;
}

std::string StopBundleCmd::getDescription()
{
    return "Stops a bundle (Parameters: bundle name).";
}

std::string StopBundleCmd::execute( IAdministrationService* const adminService, std::vector<std::string> params )
{
    adminService->stopBundle( params[0] );
    return "";
}
