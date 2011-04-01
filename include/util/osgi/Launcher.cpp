

template<
class LockType,
template <class> class CreationPolicy>
Logger& Launcher<LockType, CreationPolicy>::logger_ = LoggerFactory::getLogger( "Framework" );

template<
class LockType,
template <class> class CreationPolicy>
Launcher<LockType, CreationPolicy>::Launcher()
{
    logger_.log( Logger::LOG_DEBUG, "[Launcher#ctor] Called." );
    this->registry_ = this->createRegistry();
}

template<
class LockType,
template <class> class CreationPolicy>
Launcher<LockType, CreationPolicy>::~Launcher()
{
    logger_.log( Logger::LOG_DEBUG, "[Launcher#destructor] Called." );
    delete (this->registry_);
}


template<
class LockType,
template <class> class CreationPolicy>
IRegistry& Launcher<LockType, CreationPolicy>::getRegistry()
{
    return (*(this->registry_));
}

template<
class LockType,
template <class> class CreationPolicy>
void Launcher<LockType, CreationPolicy>::setLogLevel( Logger::LogLevel level )
{
    LoggerFactory::setLogLevel( level );
}

template<
class LockType,
template <class> class CreationPolicy>
IRegistry* Launcher<LockType, CreationPolicy>::createRegistry()
{
    logger_.log( Logger::LOG_DEBUG, "[Launcher#createRegistry] Called." );
    return new IRegistryImpl<LockType>;
}

template<
class LockType,
template <class> class CreationPolicy>
IBundleContext* Launcher<LockType, CreationPolicy>::createBundleContext( const std::string& bundleName )
{
    logger_.log( Logger::LOG_DEBUG, "[Launcher#createBundleContext] Called." );
    return new IBundleContextImpl( bundleName, (*(this->registry_)) );
}

template<
class LockType,
template <class> class CreationPolicy>
void Launcher<LockType, CreationPolicy>::start( std::vector<BundleConfiguration> &configVector )
{
    logger_.log( Logger::LOG_DEBUG, "[Launcher#start] Called." );

    std::vector<BundleConfiguration>::iterator itVectorData;
    itVectorData = configVector.begin();
    for (itVectorData = configVector.begin(); itVectorData != configVector.end(); itVectorData++)
    {
        BundleConfiguration bundleConfig = *(itVectorData);
        this->objectCreator_.setSearchConfiguration( true,
                bundleConfig.getLibraryPath(), bundleConfig.getLibraryName() );

        logger_.log( Logger::LOG_DEBUG, "[Launcher#start] Reading configuration: Library path: %1, class name: %2",
                    bundleConfig.getLibraryPath(), bundleConfig.getClassName() );

        logger_.log( Logger::LOG_DEBUG, "[Launcher#start] Loading bundle activator: Library path: %1, class name: %2",
                    bundleConfig.getLibraryPath(), bundleConfig.getClassName() );

        IBundleActivator* bundleActivator = NULL;
        try
        {
            bundleActivator = this->objectCreator_.createObject( bundleConfig.getClassName() );
        }
        catch ( ObjectCreationException &exc )
        {
            std::string msg( exc.what() );
            logger_.log( Logger::LOG_ERROR, "[Launcher#start] Error during loading bundle activator, exc: %1", msg );
            try{
                bundleActivator = this->objectCreator_.createObject( bundleConfig.getBundleName() );
            }catch ( ObjectCreationException &exc )
            {
                std::string msg( exc.what() );
                logger_.log( Logger::LOG_ERROR, "[Launcher#start] Error during loading bundle activator, exc: %1", msg );
                continue;
            }
        }

        IBundleContext* bundleCtxt = this->createBundleContext( bundleConfig.getBundleName() );
        BundleInfoBase* bundleInfo = new BundleInfo( bundleConfig.getBundleName(), false, bundleActivator, bundleCtxt );
        this->registry_->addBundleInfo( (*bundleInfo) );

        logger_.log( Logger::LOG_DEBUG, "[Launcher#start] Start bundle." );

        bundleActivator->start( bundleCtxt );
    }
}

template<
class LockType,
template <class> class CreationPolicy>
void Launcher<LockType, CreationPolicy>::startAdministrationBundle()
{
    logger_.log( Logger::LOG_DEBUG, "[Launcher#startAdministrationBundle] Called." );
    IBundleActivator* adminBundleActivator = this->objectCreator_.createObject( "sof::services::admin::AdministrationActivator" );
    IBundleContext* bundleCtxt = this->createBundleContext( "AdministrationBundle" );

    BundleInfoBase* bundleInfo = new BundleInfo( "AdministrationBundle", true, adminBundleActivator, bundleCtxt );
    this->registry_->addBundleInfo( (*bundleInfo) );

    logger_.log( Logger::LOG_DEBUG, "[Launcher#start] Start bundle." );

    AdministrationActivator* adminActivator = static_cast<AdministrationActivator*> (adminBundleActivator);
    adminActivator->setAdministrationProvider( this );
    adminActivator->start( bundleCtxt );
}

template<
class LockType,
template <class> class CreationPolicy>
void Launcher<LockType, CreationPolicy>::stop()
{
    logger_.log( Logger::LOG_DEBUG, "[Launcher#stop] Called." );
    this->registry_->removeAllBundleInfos();
}

template<
class LockType,
template <class> class CreationPolicy>
void Launcher<LockType, CreationPolicy>::startBundle( BundleConfiguration bundleConfig )
{
    logger_.log( Logger::LOG_DEBUG, "[Launcher#startBundle] Called, bundle config: %1", bundleConfig.toString() );
    std::vector<BundleConfiguration> vec;
    vec.push_back( bundleConfig );
    this->start( vec );
}

template<
class LockType,
template <class> class CreationPolicy>
void Launcher<LockType, CreationPolicy>::stopBundle( const std::string& bundleName )
{
    logger_.log( Logger::LOG_DEBUG, "[Launcher#stopBundle] Called, bundle name: %1", bundleName );
    this->registry_->removeBundleInfo( bundleName );
}

template<
class LockType,
template <class> class CreationPolicy>
std::vector<std::string> Launcher<LockType, CreationPolicy>::getBundleNames()
{
    std::vector<std::string> bundleNameVec;
    std::vector<BundleInfoBase*> vec = this->registry_->getBundleInfos();
    std::vector<BundleInfoBase*>::iterator iter;
    for ( iter = vec.begin(); iter != vec.end(); iter++ )
    {
        bundleNameVec.push_back( (*iter)->getBundleName() );
    }
    return bundleNameVec;
}

template<
class LockType,
template <class> class CreationPolicy>
std::string Launcher<LockType, CreationPolicy>::dumpBundleInfo( const std::string& bundleName )
{
    BundleInfoBase* bi = this->registry_->getBundleInfo( bundleName );
    if ( bi == 0 )
    {
        return "Bundle not available!";
    }
    else
    {
        return bi->toString();
    }
}

template<
class LockType,
template <class> class CreationPolicy>
std::string Launcher<LockType, CreationPolicy>::dumpAllBundleNames()
{
    std::vector<BundleInfoBase*> vec = this->registry_->getBundleInfos();
    std::vector<BundleInfoBase*>::iterator it;

    std::ostringstream stream;
    stream << "*** Started Bundles *** " << endl;

    for ( it = vec.begin(); it != vec.end(); it++ )
    {
        stream << (*it)->getBundleName() << endl;
    }
    stream << endl;
    return stream.str();
}

template<
class LockType,
template <class> class CreationPolicy>
BundleInfoBase& Launcher<LockType, CreationPolicy>::getBundleInfo( const std::string& bundleName )
{
    return ( * ( this->registry_->getBundleInfo( bundleName ) ) );
}
