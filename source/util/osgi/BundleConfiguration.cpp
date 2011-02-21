#include <util/osgi/BundleConfiguration.h>

#include <sstream>

using namespace izenelib::osgi;
using namespace std;

const string BundleConfiguration::NO_LIB_PATH = "";
const string BundleConfiguration::NO_LIB_NAME = "";

BundleConfiguration::BundleConfiguration( const string &bundle, const string& clName, const string &libPath, const string &libName ) :
        bundleName_( bundle ), className_(clName), libraryPath_( libPath ), libraryName_( libName )
{

}

BundleConfiguration::BundleConfiguration( const string &bundle, const string& clName ) :
        bundleName_( bundle ), className_(clName), libraryPath_( NO_LIB_PATH ), libraryName_( NO_LIB_NAME )
{

}

string BundleConfiguration::getBundleName()
{
    return this->bundleName_;
}

string BundleConfiguration::getClassName()
{
    return this->className_;
}

string BundleConfiguration::getLibraryPath()
{
    return this->libraryPath_;
}

string BundleConfiguration::getLibraryName()
{
    return this->libraryName_;
}

string BundleConfiguration::toString()
{
    ostringstream infoStream;
    infoStream << "bundleConfig={";
    infoStream << "bundleName_=" << this->bundleName_ << ", ";
    infoStream << "className_=" << this->className_ << ", ";
    infoStream << "libraryName_=" << this->libraryName_ << ", ";
    infoStream << "libraryPath_=" << this->libraryPath_ << "}";
    return infoStream.str();
}

