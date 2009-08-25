#include "ProcessOptions.h"

#include <iostream>

using namespace std;
using namespace boost;

namespace po = boost::program_options;

namespace sf1v5_dummy {

ProcessOptions::ProcessOptions() :
	controllerDescription_("Controller Options"),
			configurationProcessDescription_("Configuration Process Options"),
			idProcessDescription_("ID Process Options"),
			logProcessDescription_("Log Process Options"),
			laProcessDescription_("LA Process Options"),
			indexProcessDescription_("Index Process Options"),
			documentProcessDescription_("Document Process Options"),
			mainLightProcessDescription_("Main Process(MF-light) Options"),
			fullDescription_("Options") {
	po::options_description base;
	po::options_description hostPort;
	po::options_description controllerIpPort;
	po::options_description configFile;
	po::options_description scdParsing;
	po::options_description agentInfo;

	base.add_options() ("help", "Display help message");

	hostPort.add_options() ("host-port,H", po::value<unsigned int>(), "Port number of the host");

	agentInfo.add_options()	("agent-info,A", po::value<string>(), "agentInfo (collecion name)");

	controllerIpPort.add_options()
	("controller-ip,I", po::value<string>(), "IP address of the controller")
	("controller-port,P", po::value<unsigned int>(), "Port number of the controller");

	configFile.add_options()
	("config-file,F", po::value<string>(), "Path to the configuration file");

	scdParsing.add_options()
	("collection-name,C", po::value<string>(), "The name of the SCD document collection")
	("SCD-file,S", po::value<string>(), "File(s) that contain the documents")
	("number-of-docs,N", po::value<unsigned int>(), "Number of Documents that DocumentManager will process");

	controllerDescription_.add_options()
	("help", "Display help message")
	("controller-port,P", po::value<unsigned int>(), "Port number of the controller");

	configurationProcessDescription_.add(base).add( hostPort ).add(controllerIpPort ).add( configFile );

        idProcessDescription_.add(base).add( hostPort ).add( controllerIpPort );

        logProcessDescription_.add(base).add( hostPort ).add( controllerIpPort );

        laProcessDescription_.add(base).add( hostPort ).add( controllerIpPort );

        indexProcessDescription_.add(base).add( hostPort ).add( controllerIpPort ).add( agentInfo ) ;

        documentProcessDescription_.add(base).add( hostPort ).add( controllerIpPort ).add( scdParsing );

        mainLightProcessDescription_.add(base).add(hostPort).add( configFile ).add( scdParsing );

        mainProcessDescription_.add(base).add( hostPort ).add( controllerIpPort );

        fullDescription_.add(base).add( hostPort ).add( controllerIpPort ).add( configFile ).add( scdParsing ).add(agentInfo);
    }


    void ProcessOptions::setProcessOptions()
    {
        if( variableMap_.count("host-port") )
        {
            hostPort_ = variableMap_["host-port"].as<unsigned int>();
        }

        if( variableMap_.count("controller-ip") )
        {
            controllerIp_ = variableMap_["controller-ip"].as<string>();
        }
        
        if( variableMap_.count("agent-info") )
        {
            agentInfo_ = variableMap_["agent-info"].as<string>();
        }        

        if( variableMap_.count("controller-port") )
        {
            controllerPort_ = variableMap_["controller-port"].as<unsigned int>();
        }

        if( variableMap_.count("config-file") )
        {
            configFileName_ = variableMap_["config-file"].as<string>();
        }

        if( variableMap_.count("collection-name") )
        {
            collectionName_= variableMap_["collection-name"].as<string>();
        }

        if( variableMap_.count("SCD-file") )
        {
            scdFileName_ = variableMap_["SCD-file"].as<string>();
        }

        if( variableMap_.count("number-of-docs") )
        {
            documentCount_ = variableMap_["number-of-docs"].as<unsigned int>();
        }
    }




    bool ProcessOptions::setControllerOptions( int argc, char * argv[] )
    {
        po::store( po::parse_command_line(argc, argv, fullDescription_), variableMap_ );
        po::notify(variableMap_);

        if( variableMap_.empty() || variableMap_.count("help") || (variableMap_.size() != 1 ) )
        {
            cout << "Usage:  controller <settings (-P)>" << endl;
            cout << controllerDescription_;
            return false;
        }

        setProcessOptions();

        return true;
    }

    bool ProcessOptions::setConfigProcessOptions( int argc, char * argv[] )
    {

        po::store( po::parse_command_line(argc, argv, fullDescription_), variableMap_ );
        po::notify(variableMap_);


        if( variableMap_.empty() || variableMap_.count("help") || (variableMap_.size() != 4 ) )
        {
            cout << "Usage:  ConfigurationProcess <settings (-H, -I, -P, -F)>" << endl;
            cout << configurationProcessDescription_;
            return false;
        }

        setProcessOptions();

        return true;
    }

    bool ProcessOptions::setIdProcessOptions( int argc, char * argv[] )
    {

        po::store( po::parse_command_line(argc, argv, fullDescription_), variableMap_ );
        po::notify(variableMap_);


        if( variableMap_.empty() || variableMap_.count("help") || (variableMap_.size() != 3 ) )
        {
            cout << "Usage:  IDProcess <settings (-H, -I, -P)>" << endl;
            cout << idProcessDescription_;
            return false;
        }

        setProcessOptions();

        return true;
    }

    bool ProcessOptions::setLogProcessOptions( int argc, char * argv[] )
    {

        po::store( po::parse_command_line(argc, argv, fullDescription_), variableMap_ );
        po::notify(variableMap_);


        if( variableMap_.empty() || variableMap_.count("help") || (variableMap_.size() != 3 ) )
        {
            cout << "Usage:  LogProcess <settings (-H, -I, -P)>" << endl;
            cout << logProcessDescription_;
            return false;
        }

        setProcessOptions();

        return true;
    }

    bool ProcessOptions::setLaProcessOptions( int argc, char * argv[] )
    {

        po::store( po::parse_command_line(argc, argv, fullDescription_), variableMap_ );
        po::notify(variableMap_);


        if( variableMap_.empty() || variableMap_.count("help") || (variableMap_.size() != 3 ) )
        {
            cout << "Usage:  LAProcess <settings (-H, -I, -P)>" << endl;
            cout << laProcessDescription_;
            return false;
        }

        setProcessOptions();

        return true;
    }

    bool ProcessOptions::setIndexProcessOptions( int argc, char * argv[] )
    {

        po::store( po::parse_command_line(argc, argv, fullDescription_), variableMap_ );
        po::notify(variableMap_);

        if( variableMap_.empty() || variableMap_.count("help") || (variableMap_.size() != 4 ) )
        {
            cout << "Usage:  IndexProcess <settings (-H, -I, -P, -A)>" << endl;
            cout << indexProcessDescription_;
            return false;
        }

        setProcessOptions();

        return true;
    }

    bool ProcessOptions::setDocumentProcessOptions( int argc, char * argv[] )
    {

        po::positional_options_description p;
        p.add("SCD-file", -1);

        po::store( po::command_line_parser(argc, argv).options(fullDescription_).positional(p).run(), variableMap_ );
        po::notify(variableMap_);


        if( variableMap_.empty() || variableMap_.count("help") || (variableMap_.size() != 6 ) )
        {
            cout << "Usage:  DocumentProcess <settings (-H, -I, -P, -C, -S, -N)>" << endl;
            cout << documentProcessDescription_;
            return false;
        }

        setProcessOptions();

        return true;
    }

    bool ProcessOptions::setMainLightProcessOptions( int argc, char * argv[] )
    {

        po::positional_options_description p;
        p.add("SCD-file", -1);

        po::store( po::command_line_parser(argc, argv).options(fullDescription_).positional(p).run(), variableMap_ );
        po::notify(variableMap_);


        if( variableMap_.empty() || variableMap_.count("help") || (variableMap_.size() != 5 ) )
        {
            cout << "Usage:  sf1v5 <settings (-H -F -C, -S, -N)>" << endl;
            cout << mainLightProcessDescription_;
            return false;
        }

        setProcessOptions();

        return true;
    }

    bool ProcessOptions::setMainProcessOptions( int argc, char * argv[] )
    {
        po::store( po::parse_command_line(argc, argv, fullDescription_), variableMap_ );
        po::notify(variableMap_);


        if( variableMap_.empty() || variableMap_.count("help") || variableMap_.size() != 3 )
        {
            cout << "Usage:  MainProcess <settings (-H, -I, -P)>" << endl;
            cout << mainProcessDescription_;
            return false;
        }

        setProcessOptions();

        return true;
    }

    bool ProcessOptions::setAllOptions( int argc, char * argv[] )
    {

        po::positional_options_description p;
        p.add("SCD-file", -1);

        po::store( po::command_line_parser(argc, argv).options(fullDescription_).positional(p).run(), variableMap_ );
        po::notify(variableMap_);

        
        if( variableMap_.empty() || variableMap_.count("help") || (variableMap_.size() != 7 ) )
        {
            cout << "Usage:  exec <settings (-H, -I, -P, -F, -C, -S, -N)>" << endl;
            cout << fullDescription_;
            return false;
        }

        setProcessOptions();

        return true;
    }

}   //namespace
