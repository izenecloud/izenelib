

template<
class ThreadingModel,
template <class> class CreationPolicy>
Logger& Launcher<ThreadingModel, CreationPolicy>::logger_ = LoggerFactory::getLogger( "Framework" );

template<
class ThreadingModel,
template <class> class CreationPolicy>
Launcher<ThreadingModel, CreationPolicy>::Launcher()
{
    logger_.log( Logger::LOG_DEBUG, "[Launcher#ctor] Called." );
    this->registry_ = this->createRegistry();
}

template<
class ThreadingModel,
template <class> class CreationPolicy>
Launcher<ThreadingModel, CreationPolicy>::~Launcher()
{
    logger_.log( Logger::LOG_DEBUG, "[Launcher#destructor] Called." );
    delete (this->registry_);
}


template<
class ThreadingModel,
template <class> class CreationPolicy>
IRegistry& Launcher<ThreadingModel, CreationPolicy>::getRegistry()
{
    return (*(this->registry_));
}

template<
class ThreadingModel,
template <class> class CreationPolicy>
void Launcher<ThreadingModel, CreationPolicy>::setLogLevel( Logger::LogLevel level )
{
    LoggerFactory::setLogLevel( level );
}

template<
class ThreadingModel,
template <class> class CreationPolicy>
IRegistry* Launcher<ThreadingModel, CreationPolicy>::createRegistry()
{
    logger_.log( Logger::LOG_DEBUG, "[Launcher#createRegistry] Called." );
    return new IRegistryImpl<ThreadingModel>;
}

template<
class ThreadingModel,
template <class> class CreationPolicy>
IBundleContext* Launcher<ThreadingModel, CreationPolicy>::createBundleContext( const std::string& bundleName )
{
    logger_.log( Logger::LOG_DEBUG, "[Launcher#createBundleContext] Called." );
    return new IBundleContextImpl( bundleName, (*(this->registry_)) );
}

template<
class ThreadingModel,
template <class> class CreationPolicy>
void Launcher<ThreadingModel, CreationPolicy>::start( std::vector<BundleConfiguration> &configVector )
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

        IBundleActivator* bundleActivator;
        try
        {
            bundleActivator = this->objectCreator_.createObject( bundleConfig.getClassName() );
        }
        catch ( ObjectCreationException &exc )
        {
            std::string msg( exc.what() );
            logger_.log( Logger::LOG_ERROR, "[Launcher#start] Error during loading bundle activator, exc: %1", msg );
            continue;
        }

        IBundleContext* bundleCtxt = this->createBundleContext( bundleConfig.getBundleName() );
        ///!!Yingfeng
        ///We could add configuration reader here for IBundleContext
        BundleInfoBase* bundleInfo = new BundleInfo( bundleConfig.getBundleName(), false, bundleActivator, bundleCtxt );
        this->registry_->addBundleInfo( (*bundleInfo) );

        logger_.log( Logger::LOG_DEBUG, "[Launcher#start] Start bundle." );

        bundleActivator->start( bundleCtxt );
    }
}

template<
class ThreadingModel,
template <class> class CreationPolicy>
void Launcher<ThreadingModel, CreationPolicy>::startAdministrationBundle()
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
class ThreadingModel,
template <class> class CreationPolicy>
void Launcher<ThreadingModel, CreationPolicy>::stop()
{
    logger_.log( Logger::LOG_DEBUG, "[Launcher#stop] Called." );
    this->registry_->removeAllBundleInfos();
}

template<
class ThreadingModel,
template <class> class CreationPolicy>
void Launcher<ThreadingModel, CreationPolicy>::startBundle( BundleConfiguration bundleConfig )
{
    logger_.log( Logger::LOG_DEBUG, "[Launcher#startBundle] Called, bundle config: %1", bundleConfig.toString() );
    std::vector<BundleConfiguration> vec;
    vec.push_back( bundleConfig );
    this->start( vec );
}

template<
class ThreadingModel,
template <class> class CreationPolicy>
void Launcher<ThreadingModel, CreationPolicy>::stopBundle( const std::string& bundleName )
{
    logger_.log( Logger::LOG_DEBUG, "[Launcher#stopBundle] Called, bundle name: %1", bundleName );
    this->registry_->removeBundleInfo( bundleName );
}

template<
class ThreadingModel,
template <class> class CreationPolicy>
std::vector<std::string> Launcher<ThreadingModel, CreationPolicy>::getBundleNames()
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
class ThreadingModel,
template <class> class CreationPolicy>
std::string Launcher<ThreadingModel, CreationPolicy>::dumpBundleInfo( const std::string& bundleName )
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
class ThreadingModel,
template <class> class CreationPolicy>
std::string Launcher<ThreadingModel, CreationPolicy>::dumpAllBundleNames()
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
class ThreadingModel,
template <class> class CreationPolicy>
BundleInfoBase& Launcher<ThreadingModel, CreationPolicy>::getBundleInfo( const std::string& bundleName )
{
    return ( * ( this->registry_->getBundleInfo( bundleName ) ) );
}
