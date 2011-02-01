#include <util/osgi/BundleConfiguration.h>

#include <sstream>

using namespace izenelib::osgi;
using namespace std;

const string BundleConfiguration::NO_LIB_PATH = "";
const string BundleConfiguration::NO_LIB_NAME = "";

BundleConfiguration::BundleConfiguration( const string &bundle, const string& clName, const string &libPath, const string &libName ) :
        bundleName( bundle ), className(clName), libraryPath( libPath ), libraryName( libName )
{

}

BundleConfiguration::BundleConfiguration( const string &bundle, const string& clName ) :
        bundleName( bundle ), className(clName), libraryPath( NO_LIB_PATH ), libraryName( NO_LIB_NAME )
{

}

string BundleConfiguration::getBundleName()
{
    return this->bundleName;
}

string BundleConfiguration::getClassName()
{
    return this->className;
}

string BundleConfiguration::getLibraryPath()
{
    return this->libraryPath;
}

string BundleConfiguration::getLibraryName()
{
    return this->libraryName;
}

string BundleConfiguration::toString()
{
    ostringstream infoStream;
    infoStream << "bundleConfig={";
    infoStream << "bundleName=" << this->bundleName << ", ";
    infoStream << "className=" << this->className << ", ";
    infoStream << "libraryName=" << this->libraryName << ", ";
    infoStream << "libraryPath=" << this->libraryPath << "}";
    return infoStream.str();
}

