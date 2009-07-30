#include "main_defs.h"

#include <ProcessOptions.h>

#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <vector>
#include <iostream>
#include <sstream>


using namespace std;
using namespace sf1v5_dummy;


void createProcessThread(
        boost::thread_group & processThreadGroup,
        const string & argString,
        int (*functionPointer)(int , char ** ),
        char ** &processArgv
        )
{
    stringstream ss(argString);

    string argument;
    vector<string> argumentList;

    while( ss >> argument )
    {
        argumentList.push_back( argument );
    }

    processArgv = new char*[ argumentList.size() ];

    for( unsigned int i=0; i < argumentList.size(); i++ )
    {
        processArgv[i] = new char [ argumentList[i].length() + 1 ];

        memset( processArgv[i], 0, argumentList[i].length() + 1 );
        memcpy( processArgv[i], argumentList[i].c_str(), argumentList[i].length() );

        //cerr << "DEBUG) " << argString << " [" << i << "]: " << processArgv[i] << endl;
    }

    processThreadGroup.create_thread( boost::bind( *functionPointer, argumentList.size(), processArgv) );
}

int main( int argc, char * argv[] )
{
    sf1v5_dummy::ProcessOptions po;

    if( !po.setMainLightProcessOptions( argc, argv ) )
    {
        return 0;
    }


    char **controllerProcessArgv;
    char **laProcessArgv;
    char **indexProcessArgv;
    char **documentProcessArgv;
    char **mainProcessArgv;


    boost::thread_group processThreadGroup;


    string argString;
    char buffer[50];    //buffer for converting data types to string.

    argString = "controller -P9000";
    createProcessThread( processThreadGroup, argString,  controllerProcess, controllerProcessArgv);
    argString.clear();

    argString = "LAProcess -H1114 -Ilocalhost -P9000";
    createProcessThread( processThreadGroup, argString,  laProcess, laProcessArgv);
    argString.clear();

    argString = "IndexProcess -H1115 -Ilocalhost -P9000";
    createProcessThread( processThreadGroup, argString,  indexProcess, indexProcessArgv);
    argString.clear();

    argString = "DocumentProcess -H1116 -Ilocalhost -P9000 -C"; argString += po.getCollectionName();
    argString += " -S"; argString += po.getScdFileName();
    argString += " -N";
    memset( buffer, 0, 50 );
    sprintf( buffer, "%u", po.getNumberOfDocuments() );
    argString += buffer;
    createProcessThread( processThreadGroup, argString,  documentProcess, documentProcessArgv);
    argString.clear();

    argString = "Main -H";
    memset( buffer, 0, 50 );
    sprintf( buffer, "%u", po.getHostPort() );
    argString += buffer;
    argString += " -I localhost -P 9000 ";
    createProcessThread( processThreadGroup, argString,  mainProcess, mainProcessArgv);
    argString.clear();



    //TODO: DEALLOCATE MEMORY!!!  delete [] configProcessArgv;

    processThreadGroup.join_all();


    return 0;
}

