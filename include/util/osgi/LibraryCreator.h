#ifndef LIBRARY_CREATOR_H
#define LIBRARY_CREATOR_H

#include <string>
#include <memory>

#include <util/DynamicLibrary.h>

#include <util/osgi/util/Logger.h>
#include <util/osgi/util/LoggerFactory.h>

#include "ObjectCreationException.h"

namespace izenelib{ namespace osgi{

using namespace izenelib::util;
using namespace izenelib::osgi::logging;
/**
 * The <code>LibraryCreator</code> class implements the creation policy which
 * is used by the <code>ObjectCreator</code> class (see
 * {@link izenelib::osgi::ObjectCreator::createObjectFromLibrary})
 * in order to create objects from a library.<br>
 *
 * @author magr74
 */
template <typename BaseT>
class LibraryCreator
{
public:

    /**
     * Creates an object from a dynamic library.
     *
     * @param path
     *     The path where the dynamic library is located.
     * @param libraryName
     *     The name of the dynamic library.
     * @param className
     *     The name of the class an object is created from.
     * @return
     *     The pointer to the created object.
     */
    static BaseT* createObjectFromLibrary( const std::string &path, const std::string &libraryName, const std::string &className );

    /**
     * Returns a logger instance.
     *
     * @return
     * Returns an instance of class <code>Logger</code>.
     */
    static Logger& getLogger();
};

template <typename BaseT>
Logger& LibraryCreator<BaseT>::getLogger()
{
    static Logger& logger = LoggerFactory::getLogger( "ObjectCreation" );
    return logger;
}

template <typename BaseT>
BaseT* LibraryCreator<BaseT>::createObjectFromLibrary( const std::string &path, const std::string &libraryName, const std::string &className )
{
    typedef BaseT* (*LIBPROC) ( const char* );

    std::unique_ptr<DynamicLibrary> pDynLib(new DynamicLibrary(false));

    LIBPROC pFunc = NULL;

    std::ostringstream str;

    int pos = path.find_last_of( '/' );
    if ( pos == (int)( path.size() - 1 ) )
    {
        str << path << libraryName;
    }
    else
    {
        str << path << '/' << libraryName;
    }

    getLogger().log( Logger::LOG_DEBUG, "[LibraryCreator#createObjectFromLibrary] Loading library: '%1'", str.str() );

    try
    {
        pDynLib->load(str.str());		
    }
    catch ( std::runtime_error &exc )
    {
        pDynLib->unloadLibrary();		 
        getLogger().log( Logger::LOG_ERROR, "[LibraryCreator#createObjectFromLibrary] Error occurred during loading library: %1", exc.what() );
    }

    try
    {
        pFunc = ( LIBPROC )(pDynLib->getSymbol("createObject"));
    }
    catch ( std::runtime_error &exc)
    {
        pDynLib->unloadLibrary();
        getLogger().log( Logger::LOG_ERROR, "[LibraryCreator#createObjectFromLibrary] Error occurred during calling library entry method, %1", exc.what() );
        pFunc = NULL;
    }

    if ( pFunc == NULL )
    {
        ObjectCreationException exc( "Error during loading object from library." );
        throw exc;
    }
    return ((*pFunc)(className.c_str()));
}


}
}
#endif

