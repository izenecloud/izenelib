#include <util/osgi/admin/DumpAllBundlesCmd.h>

using namespace izenelib::osgi::admin;

DumpAllBundlesCmd::DumpAllBundlesCmd()
{
}

std::string DumpAllBundlesCmd::getName()
{
    return "dab";
}

int DumpAllBundlesCmd::getParameterNum()
{
    return 0;
}

std::string DumpAllBundlesCmd::getDescription()
{
    return "Dumps the name of all started bundles.";
}

std::string DumpAllBundlesCmd::execute( IAdministrationService* const adminService, std::vector<std::string> params )
{
    return adminService->dumpAllBundleNames();
}
