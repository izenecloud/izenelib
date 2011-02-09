#include <stdlib.h>
#include <string>
#include <iostream>

#include <util/osgi/ObjectCreator.h>
#include <util/osgi/IBundleActivator.h>
#include <util/osgi/util/LoggerFactory.h>
#include <util/osgi/util/Logger.h>


using namespace std;
using namespace izenelib::osgi;
using namespace izenelib::osgi::logging;

#ifdef WIN32

#include <windows.h>

#define DLL extern "C" __declspec(dllexport)

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call==DLL_THREAD_ATTACH)
    {
        LoggerFactory::getLogger( "Test" ).log( Logger::LOG_DEBUG, "[bundle2_dll#DllMain] Called, dll thread attach." );
    }

    if (ul_reason_for_call==DLL_THREAD_DETACH)
    {
        LoggerFactory::getLogger( "Test" ).log( Logger::LOG_DEBUG, "[bundle2_dll#DllMain] Called, dll thread detach." );
    }

    if (ul_reason_for_call==DLL_PROCESS_ATTACH)
    {
        LoggerFactory::getLogger( "Test" ).log( Logger::LOG_DEBUG, "[bundle2_dll#DllMain] Called, dll process attach." );
    }

    if (ul_reason_for_call==DLL_PROCESS_DETACH)
    {
        LoggerFactory::getLogger( "Test" ).log( Logger::LOG_DEBUG, "[bundle2_dll#DllMain] Called, dll process detach." );
    }

    LoggerFactory::getLogger( "Test" ).log( Logger::LOG_DEBUG, "[bundle2_dll#DllMain] Left." );
    return TRUE;
}


#else
#define DLL extern "C" 
#endif

DLL IBundleActivator* createObject( const char* className )
{
    ObjectCreator<IBundleActivator> OC_BUNDLE_ACTIVATOR;
    LoggerFactory::getLogger( "Test" ).log( Logger::LOG_DEBUG, "[bundle2_so#createObject] Loading instance of class %1", std::string(className) );
    return OC_BUNDLE_ACTIVATOR.createObject( className );
}

