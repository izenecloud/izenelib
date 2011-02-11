#include <iostream>

#include "../util/StringTokenizer.h"

#include <util/osgi/BundleInfo.h>
#include <util/osgi/BundleConfiguration.h>
#include <util/osgi/ConfigFileReader.h>
#include <util/osgi/admin/IAdministrationServiceImpl.h>
#include <util/osgi/admin/DumpBundleInfoCmd.h>
#include <util/osgi/admin/StartBundleCmd.h>
#include <util/osgi/admin/StartBundleFromDLLCmd.h>
#include <util/osgi/admin/StopBundleCmd.h>
#include <util/osgi/admin/StartBundlesFromFileCmd.h>
#include <util/osgi/admin/StopAllBundlesCmd.h>
#include <util/osgi/admin/DumpAllBundlesCmd.h>

using namespace izenelib::osgi;
using namespace izenelib::osgi::logging;
using namespace izenelib::osgi::admin;


Logger& IAdministrationServiceImpl::logger = LoggerFactory::getLogger( "services" );

IAdministrationServiceImpl::IAdministrationServiceImpl( IAdministrationProvider* provider ) : adminProvider( provider )
{
    logger.log( Logger::LOG_DEBUG, "[IAdministrationServiceImpl#ctor] Called." );
}

std::vector<std::string> IAdministrationServiceImpl::getBundleNames()
{
    logger.log( Logger::LOG_DEBUG, "[IAdministrationServiceImpl#getBundleNames] Called." );
    return this->adminProvider->getBundleNames();
}

std::string IAdministrationServiceImpl::dumpBundleInfo( const std::string& bundleName )
{
    logger.log( Logger::LOG_DEBUG, "[IAdministrationServiceImpl#dumpBundleInfo] Called, bundle name: %1", bundleName );
    return this->adminProvider->dumpBundleInfo( bundleName );
}

std::string IAdministrationServiceImpl::dumpAllBundleNames()
{
    logger.log( Logger::LOG_DEBUG, "[IAdministrationServiceImpl#dumpAllBundleNames] Called." );
    return this->adminProvider->dumpAllBundleNames();
}

void IAdministrationServiceImpl::stopBundle( const std::string& bundleName )
{
    logger.log( Logger::LOG_DEBUG, "[IAdministrationServiceImpl#stopBundle] Called, bundle name: %1", bundleName );
    this->adminProvider->stopBundle( bundleName );
}

void IAdministrationServiceImpl::stopAllBundles()
{
    logger.log( Logger::LOG_DEBUG, "[IAdministrationServiceImpl#stopAllBundles] Called." );
    this->adminProvider->stop();
}

void IAdministrationServiceImpl::startBundleFromDLL( const std::string& bundleName, const std::string& className, const std::string& libPath, const std::string& libName )
{
    logger.log( Logger::LOG_DEBUG, "[IAdministrationServiceImpl#startBundleFromDLL] Called, bundle name: %1, class name: %2", bundleName, className );
    BundleConfiguration bundleConf( bundleName, className, libPath, libName );
    std::vector<BundleConfiguration> vec;
    vec.push_back( bundleConf );
    this->adminProvider->start( vec );
}

void IAdministrationServiceImpl::startBundle( const std::string& bundleName, const std::string& className )
{
    logger.log( Logger::LOG_DEBUG, "[IAdministrationServiceImpl#startBundle] Called, bundle name: %1, class name: %2", bundleName, className );
    BundleConfiguration bundleConf( bundleName, className );
    std::vector<BundleConfiguration> vec;
    vec.push_back( bundleConf );
    this->adminProvider->start( vec );
}

void IAdministrationServiceImpl::startBundlesFromConfigFile( const std::string& configFile )
{
    logger.log( Logger::LOG_DEBUG, "[IAdministrationServiceImpl#startBundles] Called, configFile: %1", configFile );
    std::vector<BundleConfiguration> bundleConfVec = ConfigFileReader::readFromFile( configFile );
    this->adminProvider->start( bundleConfVec );
}

void IAdministrationServiceImpl::startConsole()
{
    logger.log( Logger::LOG_DEBUG, "[IAdministrationServiceImpl#startConsole] Called." );

    DumpBundleInfoCmd dumpBundleCmd;
    this->cmdMap[dumpBundleCmd.getName()] = &dumpBundleCmd;

    StartBundleCmd startBundleCmd;
    this->cmdMap[startBundleCmd.getName()] = &startBundleCmd;

    StopBundleCmd stopBundleCmd;
    this->cmdMap[stopBundleCmd.getName()] = &stopBundleCmd;

    StartBundleFromDLLCmd startBundleFromDLLCmd;
    this->cmdMap[startBundleFromDLLCmd.getName()] = &startBundleFromDLLCmd;

    StartBundlesFromFileCmd startBundlesFromFile;
    this->cmdMap[startBundlesFromFile.getName()] = &startBundlesFromFile;

    StopAllBundlesCmd stopAllBundlesCmd;
    this->cmdMap[stopAllBundlesCmd.getName()] = &stopAllBundlesCmd;

    DumpAllBundlesCmd dumpAllBundlesCmd;
    this->cmdMap[dumpAllBundlesCmd.getName()] = &dumpAllBundlesCmd;

    std::string input;
    std::vector<std::string> tokens;

    cout << "-------------------------------------------------------------" << endl;
    cout << "|                       SOF console                         |" << endl;
    cout << "-------------------------------------------------------------" << endl << endl;

    cout << ">";
    while ( getline(cin,input) )
    {
        tokens.clear();
        logger.log( Logger::LOG_DEBUG, "[IAdministrationServiceImpl#startConsole] Input: %1", input );

        StringTokenizer::tokenize( input, tokens );
        if ( tokens.size() >=1 )
        {
            if ( tokens[0] == "exit" )
            {
                cout << endl << "Bye." << endl;
                break;
            }

            if ( tokens[0] == "help" )
            {
                cout << endl << "Available commands:" << endl;
                map<std::string,ConsoleCommand*>::iterator iter;
                for ( iter = this->cmdMap.begin(); iter != this->cmdMap.end(); ++iter )
                {
                    cout << iter->second->getName() << "\t\t - " << iter->second->getDescription() << endl;
                }
                cout << ">";
                continue;
            }

            logger.log( Logger::LOG_DEBUG, "[IAdministrationServiceImpl#startConsole] Token: %1", tokens[0] );
            map<std::string,ConsoleCommand*>::iterator cmdIter;
            cmdIter = this->cmdMap.find(tokens[0]);
            if ( cmdIter == this->cmdMap.end() )
            {
                cout << "!Unknown command" << endl;
            }
            else
            {
                ConsoleCommand* cmd = (*cmdIter).second;
                if ( (size_t)(cmd->getParameterNum()) != (tokens.size() -1) )
                {
                    cout << "!Command requires " << cmd->getParameterNum() << " parameter(s)" << endl;
                }
                else
                {
                    tokens.erase( tokens.begin() );
                    cout << cmd->execute( this, tokens ) << endl;
                }
            }
        }
        cout << ">";
    }
}

void IAdministrationServiceImpl::stopConsole()
{
    logger.log( Logger::LOG_DEBUG, "[IAdministrationServiceImpl#stopConsole] Called." );
}

