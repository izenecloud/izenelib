#ifndef LOGGER_IMPL_H
#define LOGGER_IMPL_H

#include <string>
#include <vector>

#include "Logger.h"


namespace izenelib
{
namespace osgi
{
namespace logging
{

using namespace std;

/**
 * The <code>LoggerImpl</code> class implements the
 * <code>Logger</code> interface and is reponsible for
 * printing the log messages to the console.
 *
 * @author magr74
 */
class LoggerImpl : public Logger
{
private:

    /**
     * The name of the log channel.
     */
    string logCh;

    /**
     * Caches the possible log levels.
     */
    string levels[4];

    /**
     * The current log level.
     */
    unsigned int level;

    /**
     * Helper method for printing the log messages.
     *
     * @param messageParts
     *         A vector containing all string parts of the log message.
     *
     * @param level
     *         The log level of the log message which was set by the user.
     */
    void logMessage( const vector<string>& messageParts, LogLevel level );

    /**
     * Helper method for printing the log messages.
     *
     * @param msg
     *         The message text.
     *
     * @param messageParts
     *         A vector containing all integer parameters of the log message.
     *
     * @param level
     *         The log level of the log message which was set by the user.
     */
    void logMessage( const string& msg, const vector<int>& messageParts, LogLevel level );

    /**
     * Helper method for printing the log messages.
     *
     * @param msg
     *         The message text.
     *
     * @param messageParts
     *         A vector containing all boolean parameters of the log message.
     *
     * @param level
     *         The log level of the log message which was set by the user.
     */
    void logMessage( const string& msg, const vector<bool>& messageParts, LogLevel level );

public:

    /**
     * Creates instances of class <code>LoggerImpl</code>.
     *
     * @param channel
     *         The name of the log channel.
     */
    LoggerImpl( const string& channel );

    /**
     * Logs a message containing three string parameters.
     *
     * @param level
     *         The log level.
     *
     * @param message
     *         The log message.
     *
     * @param param1
     *         The first string parameter.
     *
     * @param param2
     *         The second string parameter.
     *
     * @param param3
     *         The third string parameter.
     */
    void log( LogLevel level, const string& message, const string& param1,
              const string& param2, const string& param3 );

    /**
     * Logs a message containing two string parameters.
     *
     * @param level
     *         The log level.
     *
     * @param message
     *         The log message.
     *
     * @param param1
     *         The first string parameter.
     *
     * @param param2
     *         The second string parameter.
     */
    void log( LogLevel channel, const string& message, const string& param1,
              const string& param2 );

    /**
     * Logs a message containing one string parameter.
     *
     * @param level
     *         The log level.
     *
     * @param message
     *         The log message.
     *
     * @param param1
     *         The first string parameter.
     */
    void log( LogLevel level, const string& message, const string& param1 );

    /**
     * Logs a message containing three integer parameters.
     *
     * @param level
     *         The log level.
     *
     * @param message
     *         The log message.
     *
     * @param param1
     *         The first integer parameter.
     *
     * @param param2
     *         The second integer parameter.
     *
     * @param param3
     *         The third integer parameter.
     */
    void log( LogLevel level, const string& message, int param1,
              int param2, int param3 );

    /**
     * Logs a message containing two integer parameters.
     *
     * @param level
     *         The log level.
     *
     * @param message
     *         The log message.
     *
     * @param param1
     *         The first integer parameter.
     *
     * @param param2
     *         The second integer parameter.
     */
    void log( LogLevel level, const string& message, int param1,
              int param2 );

    /**
     * Logs a message containing one integer parameter.
     *
     * @param level
     *         The log level.
     *
     * @param message
     *         The log message.
     *
     * @param param1
     *         The first integer parameter.
     */
    void log( LogLevel level, const string& message, int param1 );

    /**
     * Logs a message without any parameters.
     *
     * @param level
     *         The log level.
     *
     * @param message
     *         The log message.
     */
    void log( LogLevel level, const string& message );

    /**
     * Logs a message containing one boolean parameter.
     *
     * @param level
     *         The log level.
     *
     * @param message
     *         The log message.
     *
     * @param param1
     *         The first boolean parameter.
     */
    void log( LogLevel level, const string& message, bool param1 );

    /**
     * Logs a message containing two boolean parameters.
     *
     * @param level
     *         The log level.
     *
     * @param message
     *         The log message.
     *
     * @param param1
     *         The first boolean parameter.
     *
     * @param param2
     *         The second boolean parameter.
     */
    void log( LogLevel level, const string& message, bool param1, bool param2 );

    /**
     * Sets the log level. The logger checks for each
     * <code>log</code> call whether the log level of the
     * log call matches the system wide log level set
     * by this method. If it matches the log message is printed
     * to the console, otherwise the log message is not printed.
     *
     * @param level
     *     The log level (LOG_NOLOG, LOG_ERROR, LOG_TRACE, LOG_DEBUG.
     */
    void setLogLevel( Logger::LogLevel level );


};

}
}
}
#endif
