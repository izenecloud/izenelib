#ifndef CONFIGURATION_EXCEPTION_H
#define CONFIGURATION_EXCEPTION_H

#include <stdexcept>
#include <string>

#include "OSGIException.h"

namespace izenelib{namespace osgi{

/**
 * The <code>ConfigurationException</code> indicates that
 * something went wrong during reading configuration file.<br>
 * See class {@link sof::config::ConfigFileReader}.
 *
 * @author magr74
 */
class ConfigurationException : public OSGIException
{

public:

    /**
     * Creates instances of class <code>ConfigurationException</code>.
     *
     * @param msg
     *     The message text which describes the exception.
     */
    ConfigurationException( const std::string &msg );

    /**
     * Destroys instances of class <code>ConfigurationException</code>.
     *
     */
    virtual ~ConfigurationException() throw();


};

}}
#endif

