/**
 * @file    ProcessOptions.h
 * @brief   Defines a class that handles process argument given in the form of options.
 * @author  MyungHyun Lee(Kent)
 * @date    2008-12-02
 *
 * @details
 * Currently the options are only differenciated by the display message. However the arguments are parsed for all the cases.
 * I assume that a process will not use other process' options.
 */

#ifndef _SF1V5_PROCESS_OPTIONS_H_
#define _SF1V5_PROCESS_OPTIONS_H_

#include <boost/program_options.hpp>

#include <iostream>
#include <string>
#include <vector>

namespace sf1v5_dummy {

/**
 * @brief   Parses sf1v5 process' options and provides interfaces for accessing the values.
 */
class ProcessOptions {
public:

	/**
	 * @brief   sets up option descriptions for all processes
	 */
	ProcessOptions();

	/**
	 * @brief   Displays the appropriate help message for controller. 
	 *          Checks if there are options, and calls setProcessOptions() to parse and set option values.
	 */
	bool setControllerOptions(int argc, char * argv[]);

	/**
	 * @brief   Displays the appropriate help message for ConfigurationProcess. 
	 *          Checks if there are options, and calls setProcessOptions() to parse and set option values.
	 */
	bool setConfigProcessOptions(int argc, char * argv[]);

	/**
	 * @brief   Displays the appropriate help message for IdProcess. 
	 *          Checks if there are options, and calls setProcessOptions() to parse and set option values.
	 */
	bool setIdProcessOptions(int argc, char * argv[]);

	/**
	 * @brief   Displays the appropriate help message for LogProcess. 
	 *          Checks if there are options, and calls setProcessOptions() to parse and set option values.
	 */
	bool setLogProcessOptions(int argc, char * argv[]);

	/**
	 * @brief   Displays the appropriate help message for LAProcess. 
	 *          Checks if there are options, and calls setProcessOptions() to parse and set option values.
	 */
	bool setLaProcessOptions(int argc, char * argv[]);

	/**
	 * @brief   Displays the appropriate help message for IndexProcess. 
	 *          Checks if there are options, and calls setProcessOptions() to parse and set option values.
	 */
	bool setIndexProcessOptions(int argc, char * argv[]);

	/**
	 * @brief   Displays the appropriate help message for DocumentProcess. 
	 *          Checks if there are options, and calls setProcessOptions() to parse and set option values.
	 */
	bool setDocumentProcessOptions(int argc, char * argv[]);

	/**
	 * @brief   Displays the appropriate help message for main function of sf1v5 using MF-light . 
	 *          Checks if there are options, and calls setProcessOptions() to parse and set option values.
	 *  TODO: need to change name
	 */
	bool setMainLightProcessOptions(int argc, char * argv[]);

	/**
	 * @brief   Displays the appropriate help message for MainProcess. 
	 *          Checks if there are options, and calls setProcessOptions() to parse and set option values.
	 */
	bool setMainProcessOptions(int argc, char * argv[]);

	/**
	 * @brief   Displays the appropriate help message for all the options. 
	 *          Checks if there are options, and calls setProcessOptions() to parse and set option values.
	 */
	bool setAllOptions(int argc, char * argv[]);

	unsigned int getNumberOfOptions() {
		return variableMap_.size();
	}

	/**
	 * @brief   Gets the port number of the current host.
	 * @return  The port number
	 */
	unsigned int getHostPort() {
		return hostPort_;
	}

	/**
	 * @brief   Gets the IP address of the controller.
	 * @return  The IP address
	 */
	std::string getControllerIp() {
		return controllerIp_;
	}

	/**
	 * @brief   Gets the port number of the controller.
	 * @return  The port number
	 */
	unsigned int getControllerPort() {
		return controllerPort_;
	}

	/**
	 * @brief   Gets the location of the configuration file
	 * @return  The configuration file path
	 */
	std::string getConfigFileName() {
		return configFileName_;
	}

	/**
	 * @brief   Gets a Collection name.
	 * @return  A Collection name.
	 */
	std::string getCollectionName() {
		return collectionName_;
	}

	/**
	 * @brief   Gets a number of SCD files in DocumentManager.
	 * @return  SCD files name
	 */
	std::string getScdFileName() {
		return scdFileName_;
	}

	/**
	 * @brief   Gets a number of documents to process in DocumentManager.
	 * @return  number of documents to process
	 */
	unsigned int getNumberOfDocuments() {
		return documentCount_;
	}

	std::string getAgentInfo() {
		return agentInfo_;
	}

private:

	//Process all the options possible for the processes in sf1v5
	void setProcessOptions();

	/// @brief  Stores the option values
	boost::program_options::variables_map variableMap_;

	/**
	 * @brief   Description of the process options for controller.
	 */
	boost::program_options::options_description controllerDescription_;

	/**
	 * @brief   Description of the process options for ConfigurationProcess.
	 */
	boost::program_options::options_description
			configurationProcessDescription_;

	/**
	 * @brief   Description of the process options for IdProcess.
	 */
	boost::program_options::options_description idProcessDescription_;

	/**
	 * @brief   Description of the process options for LogProcess.
	 */
	boost::program_options::options_description logProcessDescription_;

	/**
	 * @brief   Description of the process options for LAProcess.
	 */
	boost::program_options::options_description laProcessDescription_;

	/**
	 * @brief   Description of the process options for IndexProcess.
	 */
	boost::program_options::options_description indexProcessDescription_;

	/**
	 * @brief   Description of the process options for DocumentProcess.
	 */
	boost::program_options::options_description documentProcessDescription_;

	/**
	 * @brief   Description of the process options for Main using MF-light.
	 */
	//TODO: RENAME
	boost::program_options::options_description mainLightProcessDescription_;

	/**
	 * @brief   Description of Main process options
	 */
	boost::program_options::options_description mainProcessDescription_;

	/**
	 * @brief   Description of the process options for all the processes.
	 */
	boost::program_options::options_description fullDescription_;

	/// @brief  The port number of the current host
	unsigned int hostPort_;

	/// @brief  The ip address of the controller
	std::string controllerIp_;

	/// @brief  The port number of the controller
	unsigned int controllerPort_;

	/// @brief  The file name (path) of the configuration file
	std::string configFileName_;

	/// @brief  The name of the Collection of the documents to be processed by DocumentManager
	std::string collectionName_;

	/// @brief  The name of the SCD file that cotains the documents
	std::string scdFileName_;

	/// @brief  The number of documents that DocumentManager will process from the SCD file
	unsigned int documentCount_;

	std::string agentInfo_;

};

} //namespace


#endif  //_SF1V5_PROCESS_OPTIONS_H_
