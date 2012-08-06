#ifndef OBJECT_CREATOR_H
#define OBJECT_CREATOR_H

#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <exception>

#include "BaseFactory.h"
#include "Factory.h"
#include "ObjectCreationException.h"
#include "NullCreator.h"

#include "util/Logger.h"
#include "util/LoggerFactory.h"

#include <boost/thread.hpp>

namespace izenelib{namespace osgi{

using namespace izenelib::osgi::logging;
/**
 * The <code>ObjectCreator</code> class allows to create instances of
 * classes by name.<p>
 *
 * Example:<p>
 *
 * ObjectCreator<IBundleActivator> OC;<br>
 * IBundleActivator* activator = OC.createObject( "TestBundleActivator" );<p>
 *
 * First an instance of the template based class <code>ObjectCreator</code> has to be
 * created where the base type of the class which will be created is specified
 * (here <code>IBundleActivator</code>).<br>
 * Afterwards the user object can be created by calling the <code>createObject</code>
 * whereas the name of the class must be specified (here 'TestBundleActivator').<br>
 * The <code>ObjectCreator</code> class needs the knowledge about what object for what
 * class name must be created. This knowledge is set by the <code>REGISTER</code> macro
 * which is always defined in 'ObjectCreator.h'.<p>
 *
 * Example:<p>
 * REGISTER("TestBundleActivator",IBundleActivator,TestBundleActivator)<p>
 *
 * The REGISTER macro registers a factory object of class <code>Factory</code> with
 * the <code>ObjectCreator</code> instance.<br>
 * Furthermore the <code>ObjectCreator</code> is able to create objects from DLLs too:<p>
 *
 * IBundleActivator* activator = OC.createObjectFromLibrary( "c:/libraries", "test.dll", "TestBundleActivator" );<p>
 *
 * @author magr74
 */

template<
typename BaseT,
template <class> class CreationPolicy = NullCreator>
class ObjectCreator
{

private:

    /**
     * The std::map which stores factory objects. The factory objects are responsible for
     * creating instances of a certain type.
     */
    static std::map< std::string,BaseFactory<BaseT>* >* instanceMap;

    static boost::mutex mutex;

    /**
     * The flag indicates whether the <code>ObjectCreator</code> instance only tries to
     * create 'local' (not from DLL) objects.
     */
    bool localSearch;

    /**
     * The DLL path.
     */
    std::string path;

    /**
     * The name of the DLL from which a object is created.
     */
    std::string dllName;


public:

    /**
     * The default constructor which creates instances of class <code>ObjectCreator</code>.
     * <code>ObjectCreator</code> instances which are created by default constructor do only
     * allow creating local objects.
     */
    ObjectCreator();

    /**
     * Creates instances of class <code>ObjectCreator</code>.
     *
     * @param localSearch
     *         Indicates whether the object creator only allows creating local objects.
     *
     * @param path
     *         The path to the DLL from which objects are created.
     *
     * @param dllName
     *         The name of the DLL from which objects are created.
     */
    ObjectCreator( bool localSearch, const std::string &path, const std::string &dllName );

    /**
     * Sets the configuration for creating objects.
     *
     * @param localSearch
     *         Indicates whether the object creator only allows creating local objects.
     *
     * @param path
     *         The path to the DLL from which objects are created.
     *
     * @param dllName
     *         The name of the DLL from which objects are created.
     */
    void setSearchConfiguration( bool localSearch, const std::string &path, const std::string &dllName );

    /**
     * Creates an object of type <code>BaseT</code>.
     *
     * @param key
     *     Defines which type of object is created (the name of the class).
     *
     * @return
     *     The created object of type <code>BaseT</code>.
     */
    BaseT* createObject( const std::string &key );

    /**
     * Provides the logger instance.
     */
    static Logger& getLogger();

    /**
     * Adds a factory object for a certain class name. This method is only called
     * by <code>REGISTER</code> macro.
     *
     * @param key
     *     The name of the class.
     *
     * @param intantiator
     *     The factory object which creates objects of classes whose name is specified by
     *     first parameter.
     */
    static void addFactory( const std::string &key, BaseFactory<BaseT>* intantiator );

    /**
     * Creates only 'local' objects of classes.
     *
     * @param key
     *     The name of the class.
     */
    static BaseT* createLocalObject( const std::string &key );

    /**
     * Creates only objects of classes which are located in a DLL.
     *
     * @param path
     *     The path to the DLL.
     *
     * @param dllName
     *     The name of the DLL where the class is located.
     *
     * @param className
     *     The name of the class which is intantiated.
     */
    static BaseT* createObjectFromLibrary( const std::string &path, const std::string &dllName, const std::string &className );

    /**
     * Returns the std::map instance which caches objects of type
     * <code>BaseFactory</code>.<br>
     * If the std::map object is not available yet, a new one
     * will be created.
     *
     * @return
     *	   The std::map instance.
     */
    static std::map< std::string,BaseFactory<BaseT>* >* getInstanceMap();

};

template<
typename BaseT,
template <class> class CreationPolicy>
Logger& ObjectCreator<BaseT,CreationPolicy>::getLogger()
{
    static Logger& logger = LoggerFactory::getLogger( "ObjectCreation" );
    return logger;
}

template<
typename BaseT,
template <class> class CreationPolicy>
std::map<std::string,BaseFactory<BaseT>* >* ObjectCreator<BaseT,CreationPolicy>::instanceMap;

template<
typename BaseT,
template <class> class CreationPolicy>
boost::mutex ObjectCreator<BaseT,CreationPolicy>::mutex;

template<
typename BaseT,
template <class> class CreationPolicy>
ObjectCreator<BaseT,CreationPolicy>::ObjectCreator() : localSearch( true ), path( "" ), dllName( "" )
{
    getLogger().log( Logger::LOG_DEBUG, "[ObjectCreator#ctor] Called" );
}

template<
typename BaseT,
template <class> class CreationPolicy>
void ObjectCreator<BaseT,CreationPolicy>::setSearchConfiguration( bool searchLocal, const std::string &pathName, const std::string &libName )
{
    getLogger().log( Logger::LOG_DEBUG, "[ObjectCreator#setSearchConfiguration] Called, local search: %1", searchLocal );

    getLogger().log( Logger::LOG_DEBUG, "[ObjectCreator#setSearchConfiguration] Called, path name: %1, lib name: %2",
                     pathName, libName );
    this->localSearch = searchLocal;
    this->path = pathName;
    this->dllName = libName;
}

template<
typename BaseT,
template <class> class CreationPolicy>
ObjectCreator<BaseT,CreationPolicy>::ObjectCreator( bool doLocalSearch, const std::string &dllPath, const std::string &dll ) :
        localSearch( doLocalSearch ), path( dllPath ), dllName( dll )
{
    getLogger().log( Logger::LOG_DEBUG, "[ObjectCreator#ctor] Called, local search: %1", doLocalSearch );

    getLogger().log( Logger::LOG_DEBUG, "[ObjectCreator#ctor] Called, dll path: %1, lib name: %2",
                     dllPath, dll );
}

template<
typename BaseT,
template <class> class CreationPolicy>
std::map<std::string,BaseFactory<BaseT>* >* ObjectCreator<BaseT,CreationPolicy>::getInstanceMap()
{
    getLogger().log( Logger::LOG_DEBUG, "[ObjectCreator#getInstanceMap] Called." );
    if ( instanceMap == 0 )
    {
        getLogger().log( Logger::LOG_DEBUG, "[ObjectCreator#getInstanceMap] Instance std::map is null, create it." );
        instanceMap = new std::map<std::string,BaseFactory<BaseT>* >;
    }
    getLogger().log( Logger::LOG_DEBUG, "[ObjectCreator#getInstanceMap] Return instance std::map." );
    return instanceMap;
}

template<
typename BaseT,
template <class> class CreationPolicy>
void ObjectCreator<BaseT,CreationPolicy>::addFactory( const std::string &key, BaseFactory<BaseT>* intantiator )
{
    boost::unique_lock<boost::mutex> lock(mutex);
    getLogger().log( Logger::LOG_DEBUG, "[ObjectCreator#addFactory] Called, key: '%1'", key );
    typename std::map< std::string,BaseFactory<BaseT>* >::iterator fit = (*getInstanceMap()).find(key);
    if(fit != getInstanceMap()->end()) delete fit->second;
    (*getInstanceMap())[key] = intantiator;
    getLogger().log( Logger::LOG_DEBUG, "[ObjectCreator#addFactory] Factory for key '%1' added.", key );
}

template<
typename BaseT,
template <class> class CreationPolicy>
BaseT* ObjectCreator<BaseT,CreationPolicy>::createObject( const std::string &key )
{
    if ( this->localSearch )
    {
        getLogger().log( Logger::LOG_DEBUG, "[ObjectCreator#createObject] Do local search, path='%1', key='%2'", path, key);
        if ( ( path == "" ) && ( dllName == "" ) )
        {
            // do only local search
            getLogger().log( Logger::LOG_DEBUG, "[ObjectCreator#createObject] Do ONLY local search." );
            return createLocalObject( key );
        }
        else
        {
            try
            {
                return createLocalObject( key );
            }
            catch ( ObjectCreationException &exc )
            {
                getLogger().log( Logger::LOG_DEBUG, "[ObjectCreator#createObject] Local search failed, load from DLL." );
                return createObjectFromLibrary( this->path, this->dllName, key );
            }
        }
    }
    else
    {
        getLogger().log( Logger::LOG_DEBUG, "[ObjectCreator#createObject] Do NOT local search, but load directly from DLL." );
        // search in DLL for loading object
        return createObjectFromLibrary( this->path, this->dllName, key );
    }
}

template<
typename BaseT,
template <class> class CreationPolicy>
BaseT* ObjectCreator<BaseT,CreationPolicy>::createLocalObject( const std::string &key )
{
    getLogger().log( Logger::LOG_DEBUG, "[ObjectCreator#createLocalObject] Called, key: %1", key );
    BaseFactory<BaseT>* intantiator = ( *(ObjectCreator<BaseT,NullCreator>::getInstanceMap()) )[key];
    if ( intantiator == 0 )
    {
        getLogger().log( Logger::LOG_ERROR, "[ObjectCreator#createLocalObject] No intantiator for class available." );
        ObjectCreationException exc( "No intantiator for class available." );
        throw exc;
    }
    return intantiator->newInstance();
}

template<
typename BaseT,
template <class> class CreationPolicy>
BaseT* ObjectCreator<BaseT,CreationPolicy>::createObjectFromLibrary( const std::string &path, const std::string &dllName, const std::string &className )
{
    getLogger().log( Logger::LOG_DEBUG, "[ObjectCreator#createObjectFromLibrary] Called, DLL name: %1, class name: %2", dllName, className );
    return CreationPolicy<BaseT>::createObjectFromLibrary( path, dllName, className );
}

#define REGISTER_BUNDLE_ACTIVATOR_CLASS(key,subType) REGISTER_CLASS(key,IBundleActivator,subType)

#define DYNAMIC_REGISTER_BUNDLE_ACTIVATOR_CLASS(key,subType) \
    ObjectCreator<IBundleActivator,NullCreator>::addFactory( key, new Factory<IBundleActivator,subType> );

#define REGISTER_CLASS(key,baseType,subType) \
class Register##baseType##with##subType \
    { \
        public: \
            Register##baseType##with##subType () \
            {\
            ObjectCreator<baseType,NullCreator>::addFactory( key, new Factory<baseType,subType> );\
            } \
    } Register##baseType##with##subType##Instance; \
 
}}
#endif

