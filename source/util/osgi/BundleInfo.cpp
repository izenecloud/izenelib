#include <util/osgi/BundleInfo.h>

#include <sstream>

using namespace std;

using namespace izenelib::osgi;
using namespace izenelib::osgi::logging;

BundleInfo::BundleInfo( const string& bdleName, bool isSOFBundle, IBundleActivator* act, IBundleContext::ConstPtr bundleCtxt ) 
    :BundleInfoBase( bdleName, isSOFBundle, bundleCtxt ),
     activator(act)
{
    logger.log( Logger::LOG_DEBUG, "[BundleInfo#ctor] Called." );
}

BundleInfo::~BundleInfo()
{
    logger.log( Logger::LOG_DEBUG, "[BundleInfo#destructor] Called." );
}


IBundleActivator* BundleInfo::getBundleActivator()
{
    return this->activator;
}

