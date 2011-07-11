#include <util/osgi/util/LoggerImpl.h>

#include <util/osgi/util/Logger.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <climits> // UINT_MAX

using namespace std;
using namespace izenelib::osgi::logging;


LoggerImpl::LoggerImpl( const string& channel ):logCh(channel)
{
    this->levels[1] = "ERROR";
    this->levels[2] = "TRACE";
    this->levels[3] = "DEBUG";
    this->level = UINT_MAX;
}

void LoggerImpl::log( LogLevel level, const string& message, int param1,
                      int param2, int param3 )
{
    if ( this->level >= static_cast<unsigned int>(level) )
    {
        vector<int> ints;
        ints.push_back( param1 );
        ints.push_back( param2 );
        ints.push_back( param3 );
        this->logMessage( message, ints, level );
    }
}

void LoggerImpl::log( LogLevel level, const string& message, int param1,
                      int param2 )
{
    if ( this->level >= static_cast<unsigned int>(level) )
    {
        vector<int> ints;
        ints.push_back( param1 );
        ints.push_back( param2 );
        this->logMessage( message, ints, level );
    }
}

void LoggerImpl::log( LogLevel level, const string& message, int param1 )
{
    if ( this->level >= static_cast<unsigned int>(level) )
    {
        vector<int> ints;
        ints.push_back( param1 );
        this->logMessage( message, ints, level );
    }
}

void LoggerImpl::log( LogLevel level, const string& message, const string& param1,
                      const string& param2, const string& param3 )
{
    if ( this->level >= static_cast<unsigned int>(level) )
    {
        vector<string> strs;
        strs.push_back( message );
        strs.push_back( param1 );
        strs.push_back( param2 );
        strs.push_back( param3 );
        this->logMessage( strs, level );
    }
}

void LoggerImpl::log( LogLevel level, const string& message, const string& param1,
                      const string& param2 )
{
    if ( this->level >= static_cast<unsigned int>(level) )
    {
        vector<string> strs;
        strs.push_back( message );
        strs.push_back( param1 );
        strs.push_back( param2 );
        this->logMessage( strs, level );
    }
}

void LoggerImpl::log( LogLevel level, const string& message, const string& param1 )
{
    if ( this->level >= static_cast<unsigned int>(level) )
    {
        vector<string> strs;
        strs.push_back( message );
        strs.push_back( param1 );
        this->logMessage( strs, level );
    }
}

void LoggerImpl::log( LogLevel level, const string& message, bool param1,
                      bool param2 )
{
    if ( this->level >= static_cast<unsigned int>(level) )
    {
        vector<bool> bools;
        bools.push_back( param1 );
        bools.push_back( param2 );
        this->logMessage( message, bools, level );
    }
}

void LoggerImpl::log( LogLevel level, const string& message, bool param1 )
{
    if ( this->level >= static_cast<unsigned int>(level) )
    {
        vector<bool> bools;
        bools.push_back( param1 );
        this->logMessage( message, bools, level );
    }
}

void LoggerImpl::log( LogLevel level, const string& message )
{
    if ( this->level >= static_cast<unsigned int>(level) )
    {
        vector<string> strs;
        strs.push_back( message );
        this->logMessage( strs, level );
    }
}

void LoggerImpl::logMessage( const vector<string>& messageParts, LogLevel level )
{
    string message = messageParts[0];

    for ( unsigned int i=1; i<messageParts.size(); i++ )
    {
        ostringstream strStream;
        strStream << "%" << i;
        int pos = message.find( strStream.str(), 0 );
        if ( pos == -1 )
        {
            continue;
        }
        else
        {
            message = message.replace( pos, 2, messageParts[i] );
        }
    }
    ostringstream logStream;
    logStream << "<" << this->logCh << "> <" <<
    levels[level] << "> " << message;
    //cout << logStream.str() << endl;
}

void LoggerImpl::logMessage( const string& msgStr, const vector<int>& messageParts, LogLevel level )
{
    string msg = msgStr;
    for ( unsigned int i=0; i<messageParts.size(); i++ )
    {
        ostringstream strStream;
        strStream << "%" << (i+1);

        ostringstream intToStrStream;
        intToStrStream << messageParts[i];

        int pos = msg.find( strStream.str(), 0 );
        if ( pos == -1 )
        {
            continue;
        }
        else
        {
            msg = msg.replace( pos, 2, intToStrStream.str() );
        }
    }
    ostringstream logStream;
    logStream << "<" << this->logCh << "> <" <<
    levels[level] << "> " << msg;
    //cout << logStream.str() << endl;
}

void LoggerImpl::logMessage( const string& msgStr, const vector<bool>& messageParts, LogLevel level )
{
    string msg = msgStr;
    for ( unsigned int i=0; i<messageParts.size(); i++ )
    {
        ostringstream strStream;
        strStream << "%" << (i+1);

        ostringstream boolToStrStream;
        boolToStrStream << messageParts[i];

        int pos = msg.find( strStream.str(), 0 );
        if ( pos == -1 )
        {
            continue;
        }
        else
        {
            msg = msg.replace( pos, 2, boolToStrStream.str() );
        }
    }
    ostringstream logStream;
    logStream << "<" << this->logCh << "> <" <<
    levels[level] << "> " << msg;
    //cout << logStream.str() << endl;
}


void LoggerImpl::setLogLevel( Logger::LogLevel logLevel )
{
    this->level = logLevel;
}


