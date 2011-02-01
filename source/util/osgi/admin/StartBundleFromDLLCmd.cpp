#include <util/osgi/admin/StartBundleFromDLLCmd.h>

using namespace izenelib::osgi::admin;

StartBundleFromDLLCmd::StartBundleFromDLLCmd()
{
}

std::string StartBundleFromDLLCmd::getName()
{
    return "stbdll";
}

int StartBundleFromDLLCmd::getParameterNum()
{
    return 4;
}

std::string StartBundleFromDLLCmd::getDescription()
{
    return "Starts a bundle from DLL (Parameters: bundle name, class name, lib path, lib name).";
}

std::string StartBundleFromDLLCmd::execute( IAdministrationService* const adminService, std::vector<std::string> params )
{
    adminService->startBundleFromDLL( params[0], params[1], params[2], params[3] );
    return "";
}
