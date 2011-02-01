#ifndef LOGGER_FACTORY_H
#define LOGGER_FACTORY_H

#include <string>
#include <map>

#include "Logger.h"

using namespace std;


namespace izenelib
{
namespace osgi
{
namespace logging
{

/**
 * The <code>LoggerFactory</code> class creates objects
 * implementing the <code>Logger</code> interface.
 *
 * @author magr74
 */
class LoggerFactory
{
private:

    /**
     * The constructor is defined as private in order to
     * avoid to being called. Only the static methods
     * of this class ought to be used.
     */
    LoggerFactory();

    /**
     * Caches all created <code>Logger</code>
     * instances.
     */
    static map<string,Logger*>* loggerMap;

    /**
     * Defines the current log level which is valid
     * for all logger instances.
     */
    static Logger::LogLevel level;

public:

    /**
     * Creates a <code>Logger</code> object.
     *
     * @param logChannel
     *         Defines name of the log channel. This can be
     *         the class name where the logger instance is used
     *         for example.
     *
     * @return
     *         Returns object of type <code>Logger</code>.
     */
    static Logger& getLogger( const string& logChannel );

    /**
     * The log level which defines what kind of log messages
     * are printed out to the console (error, debub or trace messages).
     * This method sets the log level for all existing loggers.
     *
     * @param level
     *     The log level (LOG_NOLOG, LOG_ERROR, LOG_TRACE, LOG_DEBUG).
     */
    static void setLogLevel( Logger::LogLevel level );
};

}
}
}
#endif
