#ifndef OSGI_EXCEPTION_H
#define OSGI_EXCEPTION_H

#include <stdexcept>
#include <string>

namespace izenelib{namespace osgi{

/**
 * The <code>OSGIException</code> represents the base class of
 * all SOF related exceptions.
 *
 * @author magr74
 */
class OSGIException : public std::exception
{
protected:

    /**
     * The message which describes the occured exception.
     */
    std::string message;

public:

    /**
     * Creates instances of class <code>OSGIException</code>.
     *
     * @param msg
     *     The message text which describes the exception.
     */
    OSGIException( const std::string &msg );

    /**
     * Destroys the exception instance.
     */
    virtual ~OSGIException() throw();

    /**
     * Returns the message text.
     */
    virtual const char* what() const throw();
};

}}
#endif

