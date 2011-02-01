#include <util/osgi/admin/DumpBundleInfoCmd.h>

using namespace izenelib::osgi::admin;

DumpBundleInfoCmd::DumpBundleInfoCmd()
{
}

std::string DumpBundleInfoCmd::getName()
{
    return "dbi";
}

int DumpBundleInfoCmd::getParameterNum()
{
    return 1;
}

std::string DumpBundleInfoCmd::getDescription()
{
    return "Dumps the bundle data which are registered services, used services, registered listeners (Parameters: bundle name).";
}

std::string DumpBundleInfoCmd::execute( IAdministrationService* const adminService, std::vector<std::string> params )
{
    return adminService->dumpBundleInfo( params[0] );
}
