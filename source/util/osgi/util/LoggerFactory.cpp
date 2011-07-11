#include <util/osgi/util/LoggerFactory.h>
#include <util/osgi/util/Logger.h>
#include <util/osgi/util/LoggerImpl.h>

#include <iostream>

using namespace izenelib::osgi::logging;
using namespace std;

map<string,Logger*>* LoggerFactory::loggerMap;
Logger::LogLevel LoggerFactory::level = Logger::LOG_ERROR;

LoggerFactory::LoggerFactory()
{
}

Logger& LoggerFactory::getLogger( const string& logChannel )
{
    if ( loggerMap == 0 )
    {
        loggerMap = new map<string,Logger*>;
    }
    Logger* log = (*loggerMap)[logChannel];
    if ( log == 0 )
    {
        log = new LoggerImpl( logChannel );
        (*loggerMap)[logChannel] = log;
        (*log).setLogLevel( level );
    }
    return (*log);
}

void LoggerFactory::setLogLevel( Logger::LogLevel logLevel )
{
    if ( loggerMap == 0 )
    {
        return;
    }

    level = logLevel;
    map<string,Logger*>::iterator iter;
    for ( iter = loggerMap->begin(); iter != loggerMap->end(); ++iter )
    {
        Logger* log = (*loggerMap)[iter->first];
        log->setLogLevel( level );
    }
}
