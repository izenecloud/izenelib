template<class ThreadingModel>
Logger& IRegistryImpl<ThreadingModel>::logger = LoggerFactory::getLogger( "Framework" );

template<class ThreadingModel>
IRegistryImpl<ThreadingModel>::~IRegistryImpl()
{
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#destructor] Called." );

    std::map<std::string, std::vector<ServiceInfoPtr>* >::iterator serviceInfoMapIt = this->serviceInfoMap.begin();
    for( ; serviceInfoMapIt != this->serviceInfoMap.end(); ++serviceInfoMapIt)
        delete serviceInfoMapIt->second;
    
    std::map<std::string, std::vector<ServiceListenerInfoPtr>* >::iterator serviceListenerMapIt = this->serviceListenerMap.begin();
    for( ; serviceListenerMapIt != this->serviceListenerMap.end(); ++serviceListenerMapIt)
        delete serviceListenerMapIt->second;

}

template<class ThreadingModel>
void IRegistryImpl<ThreadingModel>::addBundleInfo( BundleInfoBase& bundleInfo )
{
    // !!! synchronized access !!!
    typename ThreadingModel::Lock lock;

    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#addBundleInfo] Called, bundleName: %1", bundleInfo.getBundleName() );
    this->bundleInfoMap[bundleInfo.getBundleName()] = &bundleInfo;

    // Storing the starting order of the bundles
    this->bundleNames.push_back( bundleInfo.getBundleName() );
}

template<class ThreadingModel>
BundleInfoBase* IRegistryImpl<ThreadingModel>::getBundleInfo( const std::string &bundleName )
{
    // !!! synchronized access !!!
    typename ThreadingModel::Lock lock;

    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#getBundleInfo] Called, bundleName: %1", bundleName );
    std::map<std::string,BundleInfoBase*>::iterator iter = this->bundleInfoMap.find( bundleName );
    if ( iter != this->bundleInfoMap.end() )
    {
        logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#getBundleInfo] BundleInfo object found." );
        return iter->second;
    }
    else
    {
        logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#getBundleInfo] BundleInfo object NOT found." );
        return 0;
    }
}

template<class ThreadingModel>
std::vector<BundleInfoBase*> IRegistryImpl<ThreadingModel>::getBundleInfos()
{
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#getBundleInfos] Called." );
    // !!! synchronized access !!!
    typename ThreadingModel::Lock lock;

    std::map<std::string, BundleInfoBase*>::iterator it;
    std::vector<BundleInfoBase*> vec;

    for ( it = this->bundleInfoMap.begin(); it != this->bundleInfoMap.end(); ++it )
    {
        vec.push_back( it->second );
    }
    return vec;
}

template<class ThreadingModel>
void IRegistryImpl<ThreadingModel>::removeAllBundleInfos()
{
    // !!! synchronized access !!!
    typename ThreadingModel::Lock lock;

    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#removeAllBundleInfos] Called." );

    std::vector<std::string>::reverse_iterator strIterator;
    for ( strIterator = this->bundleNames.rbegin(); strIterator != this->bundleNames.rend(); ++strIterator )
    {
        logger.log( Logger::LOG_DEBUG, "[Registry#removeAllBundleInfos] Remove bundle: %1", (*strIterator) );
        this->removeBundleInfo( (*strIterator) );
    }
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#removeAllBundleInfos] Left." );
}

template<class ThreadingModel>
void IRegistryImpl<ThreadingModel>::removeBundleInfo( const std::string &bundleName )
{
    // !!! synchronized access !!!
    typename ThreadingModel::Lock lock;

    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#removeBundleInfo] Called, bundleName: %1", bundleName );

    std::map<std::string,BundleInfoBase*>::iterator bundleInfoMapIt = this->bundleInfoMap.find(bundleName);
    if(bundleInfoMapIt == this->bundleInfoMap.end())
        return;

    BundleInfoBase* bi = this->bundleInfoMap[bundleName];

    if ( bi->isFrameworkBundle() )
    {
        logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#removeBundleInfo] Framework bundles are not stopped." );
        return;
    }

    this->deleteActivator( (*bi) );

    bi->removeAllUsedServices();

    std::vector<ServiceInfoPtr> serviceInfos = bi->getRegisteredServices();
    std::vector<ServiceInfoPtr>::iterator iter;
    for ( iter = serviceInfos.begin(); iter != serviceInfos.end(); iter++ )
    {
        logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#removeBundleInfo] Deregister service: %1", (*iter)->getServiceName() );
        this->removeServiceInfo( bundleName, (*iter) );
    }

    std::vector<ServiceListenerInfoPtr> serviceListenerInfos = bi->getRegisteredListeners();
    std::vector<ServiceListenerInfoPtr>::iterator listenerIter;
    for ( listenerIter = serviceListenerInfos.begin(); listenerIter != serviceListenerInfos.end(); listenerIter++ )
    {
        logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#removeBundleInfo] Remove listener: %1", (*listenerIter)->getServiceName() );
        this->removeServiceListener( bundleName, (*listenerIter) );
    }

    delete (bi->getBundleContext());
    delete bi;
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#removeBundleInfo] Erase bundle info." );
    this->bundleInfoMap.erase( bundleName );
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#removeBundleInfo] Left" );
}

template<class ThreadingModel>
void IRegistryImpl<ThreadingModel>::stopActivator( const BundleInfoBase& bi )
{
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#stopActivator] Called." );
    BundleInfoBase* info = const_cast<BundleInfoBase*>( &bi );
    BundleInfo* bundleInfo = dynamic_cast<BundleInfo*>( info );
    IBundleActivator* act = bundleInfo->getBundleActivator();
    act->stop( bundleInfo->getBundleContext() );
}

template<class ThreadingModel>
void IRegistryImpl<ThreadingModel>::deleteActivator( const BundleInfoBase& bi )
{
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#deleteActivator] Called." );
    BundleInfoBase* info = const_cast<BundleInfoBase*>( &bi );
    BundleInfo* bundleInfo = dynamic_cast<BundleInfo*>( info );
    IBundleActivator* act = bundleInfo->getBundleActivator();
    act->stop( bundleInfo->getBundleContext() );
    delete act;
}

template<class ThreadingModel>
IServiceRegistration::ConstPtr IRegistryImpl<ThreadingModel>::addServiceInfo( const std::string& bundleName, ServiceInfoPtr serviceInfo )
{
    // !!! synchronized access !!!
    typename ThreadingModel::Lock lock;

    std::string serviceName = serviceInfo->getServiceName();
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#addServiceInfo] Called, bundle name: %1, service name: %2", bundleName, serviceName );

    this->addToServiceInfoVector( bundleName, serviceName, serviceInfo );

    // adding ServiceInfo object to BundleInfo object
    this->addRegisteredServiceToBundleInfo( bundleName, serviceInfo );

    // notifying all Listeners about new registered service
    std::vector<ServiceListenerInfoPtr>* serviceListenerInfoVec = this->getServiceListenerInfoVector( serviceName );
    this->notifyListenersAboutRegisteredService( bundleName, serviceInfo, serviceListenerInfoVec, serviceName );
    return this->createServiceRegistrationObject( bundleName, serviceInfo );
}

template<class ThreadingModel>
IServiceRegistration::ConstPtr IRegistryImpl<ThreadingModel>::createServiceRegistrationObject( const std::string& bundleName, ServiceInfoPtr serviceInfo )
{
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#createServiceRegistrationObject] Called" );
    return new IServiceRegistrationImpl( bundleName, (*this), serviceInfo );
}

template<class ThreadingModel>
void IRegistryImpl<ThreadingModel>::removeServiceInfo( const std::string& bundleName, ServiceInfoPtr serviceInfo )
{
    // !!! synchronized access !!!
    typename ThreadingModel::Lock lock;

    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#removeServiceInfo] Called, serviceInfo: %1",
                serviceInfo->toString() );
    this->removeFromServiceInfoVector( serviceInfo );
    std::vector<ServiceListenerInfoPtr>* serviceListenerInfoVec = this->getServiceListenerInfoVector( serviceInfo->getServiceName() );
    this->notifyListenersAboutDeregisteredService( bundleName, serviceInfo, serviceListenerInfoVec );
    this->removeDeregisteredServiceFromBundleInfo( bundleName, serviceInfo );
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#removeServiceInfo] Left." );
}

template<class ThreadingModel>
void IRegistryImpl<ThreadingModel>::addRegisteredServiceToBundleInfo( const std::string& bundleName, ServiceInfoPtr serviceInfo )
{
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#addRegisteredServiceToBundleInfo] Called, bundle name: %1, service info: %2",
                bundleName, serviceInfo->toString() );
    BundleInfoBase* bundleInfo = this->getBundleInfo( bundleName );
    if ( bundleInfo == 0 )
    {
        logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#addRegisteredServiceToBundleInfo] No bundle info available, do nothing." );
    }
    else
    {
        logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#addRegisteredServiceToBundleInfo] Add registered service to bundle info." );
        bundleInfo->addRegisteredService( serviceInfo );
    }
}

template<class ThreadingModel>
void IRegistryImpl<ThreadingModel>::removeDeregisteredServiceFromBundleInfo( const std::string& bundleName, ServiceInfoPtr serviceInfo )
{
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#removeDeregisteredServiceFromBundleInfo] Called, bundle name: %1, service info: %2",
                bundleName, serviceInfo->toString() );
    BundleInfoBase* bundleInfo = this->getBundleInfo( bundleName );
    if ( bundleInfo == 0 )
    {
        logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#removeDeregisteredServiceFromBundleInfo] No bundle info available, do nothing." );
    }
    else
    {
        logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#removeDeregisteredServiceFromBundleInfo] Remove deregistered service from bundle info." );
        bundleInfo->removeDeregisteredService( serviceInfo );
    }
}

template<class ThreadingModel>
void IRegistryImpl<ThreadingModel>::addUsedService( const std::string& bundleName, ServiceInfoPtr serviceInfo )
{
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#addUsedService] Called, bundle name: %1, service info: %2",
                bundleName, serviceInfo->toString() );
    BundleInfoBase* bundleInfo = this->getBundleInfo( bundleName );
    if ( bundleInfo == 0 )
    {
        logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#addUsedService] BundleInfo object does not exist (null), do nothing." );
    }
    else
    {
        bundleInfo->addUsedService( serviceInfo );
    }
}

template<class ThreadingModel>
void IRegistryImpl<ThreadingModel>::removeUsedService( const std::string& bundleName, ServiceInfoPtr serviceInfo )
{
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#removeUsedService] Called, bundle name: %1, service info: %2",
                bundleName, serviceInfo->toString() );
    BundleInfoBase* bundleInfo = this->getBundleInfo( bundleName );
    if ( bundleInfo != 0 )
    {
        bundleInfo->removeUsedService( serviceInfo );
    }
    else
    {
        logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#removeUsedService] BundleInfo is null, do nothing." );
    }
}

template<class ThreadingModel>
void IRegistryImpl<ThreadingModel>::addToServiceInfoVector( const std::string& bundleName, const std::string& serviceName, ServiceInfoPtr serviceInfo )
{
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#addToServiceInfoVector] Called, bundle name: %1, service name: %2", bundleName, serviceName );

    BundleInfoBase* bundleInfo = this->getBundleInfo( bundleName );
    if ( bundleInfo == 0 )
    {
        logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#addToServiceInfoVector] No bundle info available, do not add service." );
        return;
    }

    std::vector<ServiceInfoPtr>* serviceVec = this->getServiceInfo( serviceName );

    serviceVec->push_back( serviceInfo );

    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#addToServiceInfoVector] Left." );
}

template<class ThreadingModel>
void IRegistryImpl<ThreadingModel>::removeFromServiceInfoVector( ServiceInfoPtr serviceInfo )
{
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#removeFromServiceInfoVector] Called, service name: %1", serviceInfo->getServiceName() );

    std::vector<ServiceInfoPtr>* vec = this->getServiceInfo( serviceInfo->getServiceName() );

    std::vector<ServiceInfoPtr>::iterator iter;

    for ( iter = vec->begin(); iter != vec->end(); iter++ )
    {
        // TODO: check why following statement can not be used (ServiceInfo class is called)
        //if ( (*(*iter)) == (*serviceInfo) )

        if ( (*iter)->equals( ( (*((*iter).GetRawPointer())) ), (*(serviceInfo.GetRawPointer())) ) )
        {
            logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#removeFromServiceInfoVector] Service was found in ServiceInfo std::vector." );
            iter = vec->erase( iter );
            break;
        }
    }
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#removeFromServiceInfoVector] Left." );
}


template<class ThreadingModel>
void IRegistryImpl<ThreadingModel>::notifyListenersAboutRegisteredService( const std::string& bundleName, std::vector<ServiceInfoPtr>* serviceVec, std::vector<ServiceListenerInfoPtr>* serviceListenerInfoVec, const std::string& serviceName )
{
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#notifyListenersAboutRegisteredService] Called." );

    std::vector<ServiceListenerInfoPtr>::iterator listenerIter;
    for ( listenerIter = serviceListenerInfoVec->begin(); listenerIter != serviceListenerInfoVec->end(); listenerIter++ )
    {
        std::vector<ServiceInfoPtr>::iterator serviceIter;
        for ( serviceIter = serviceVec->begin(); serviceIter != serviceVec->end(); serviceIter++ )
        {
            bool interested = this->callServiceListenerObject( (*listenerIter), (*serviceIter), ServiceEvent::REGISTER );

            if ( interested )
            {
                logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#notifyListenersAboutRegisteredService] Service listener is interested in registered service '%1'.",
                            (*serviceIter)->getServiceName() );
                this->addUsedService( bundleName, (*serviceIter) );
            }
            else
            {
                logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#notifyListenersAboutRegisteredService] Service listener is NOT interested in registered service '%1'.",
                            (*serviceIter)->getServiceName() );
            }
        }
    }

    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#notifyListenersAboutRegisteredService] Left." );
}

template<class ThreadingModel>
void IRegistryImpl<ThreadingModel>::notifyListenersAboutRegisteredService( const std::string& bundleName, ServiceInfoPtr serviceInfo, std::vector<ServiceListenerInfoPtr>* serviceListenerInfoVec, const std::string& serviceName )
{

    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#notifyListenersAboutRegisteredService] Called." );

    std::vector<ServiceListenerInfoPtr>::iterator listenerIter;
    for ( listenerIter = serviceListenerInfoVec->begin(); listenerIter != serviceListenerInfoVec->end(); listenerIter++ )
    {
        bool interested = this->callServiceListenerObject( (*listenerIter), serviceInfo, ServiceEvent::REGISTER );

        if ( interested )
        {
            logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#notifyListenersAboutRegisteredService] Service listener is interested in registered service '%1'.",
                        serviceInfo->getServiceName() );
            this->addUsedService( (*listenerIter)->getBundleName(), serviceInfo );
        }
        else
        {
            logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#notifyListenersAboutRegisteredService] Service listener is NOT interested in registered service '%1'.",
                        serviceInfo->getServiceName() );
        }
    }

    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#notifyListenersAboutRegisteredService] Left." );

}

template<class ThreadingModel>
void IRegistryImpl<ThreadingModel>::notifyListenerAboutRegisteredService( const std::string& bundleName, std::vector<ServiceInfoPtr>* serviceVec, const ServiceListenerInfoPtr serviceListenerInfo, const std::string& serviceName )
{
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#notifyListenerAboutRegisteredService] Called." );

    std::vector<ServiceInfoPtr>::iterator serviceIter;
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#notifyListenerAboutRegisteredService] Iterate through service info std::vector." );
    for ( serviceIter = serviceVec->begin(); serviceIter != serviceVec->end(); serviceIter++ )
    {
        logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#notifyListenerAboutRegisteredService] Service name: %1",
                    (*serviceIter)->getServiceName() );
        bool interested = this->callServiceListenerObject( serviceListenerInfo, (*serviceIter), ServiceEvent::REGISTER );
        if ( interested )
        {
            logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#notifyListenerAboutRegisteredService] Service listener is interested in registered service '%1'.",
                        (*serviceIter)->getServiceName() );
            this->addUsedService( bundleName, (*serviceIter) );
        }
        else
        {
            logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#notifyListenerAboutRegisteredService] Service listener is NOT interested in registered service '%1'.",
                        (*serviceIter)->getServiceName() );
        }
    }


    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#notifyListenerAboutRegisteredService] Left." );
}

template<class ThreadingModel>
void IRegistryImpl<ThreadingModel>::notifyListenersAboutDeregisteredService( const std::string& bundleName, ServiceInfoPtr serviceInfo, std::vector<ServiceListenerInfoPtr>* serviceListenerInfoVec )
{
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#notifyListenersAboutDeregisteredService] Called." );

    std::vector<ServiceListenerInfoPtr>::iterator listenerIter;
    for ( listenerIter = serviceListenerInfoVec->begin(); listenerIter != serviceListenerInfoVec->end(); listenerIter++ )
    {
        bool interested = this->callServiceListenerObject( (*listenerIter), serviceInfo, ServiceEvent::UNREGISTER );

        if ( interested )
        {
            logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#notifyListenersAboutDeregisteredService] Listener is interested in deregistered service '%1'.", serviceInfo->getServiceName() );
            this->removeUsedService( (*listenerIter)->getBundleName(), serviceInfo );
        }
        else
        {
            logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#notifyListenersAboutDeregisteredService] Listener is NOT interested in deregistered service '%1'.", serviceInfo->getServiceName() );
        }

    }

    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#notifyListenersAboutDeregisteredService] Left." );
}

template<class ThreadingModel>
std::vector<ServiceInfoPtr>* IRegistryImpl<ThreadingModel>::getServiceInfo( const std::string& serviceName )
{
    // !!! synchronized access !!!
    typename ThreadingModel::Lock lock;

    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#getServiceInfo] Called, service name: %1", serviceName );
    std::vector<ServiceInfoPtr>* vec = this->serviceInfoMap[serviceName];
    if ( vec == 0 )
    {
        logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#getServiceInfo] ServiceInfo std::vector is null, create one." );
        vec = new std::vector<ServiceInfoPtr>;
        this->serviceInfoMap[serviceName] = vec;
    }
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#getServiceInfo] Left." );
    return vec;
}

template<class ThreadingModel>
std::vector<ServiceListenerInfoPtr>* IRegistryImpl<ThreadingModel>::getServiceListenerInfoVector( const std::string& serviceName )
{
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#getServiceListenerInfoVector] Called, service name: %1", serviceName );
    std::vector<ServiceListenerInfoPtr>* vec = this->serviceListenerMap[serviceName];
    if ( vec == 0 )
    {
        logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#getServiceListenerInfoVector] ServiceListenerInfo std::vector is null, create one." );
        vec = new std::vector<ServiceListenerInfoPtr>;
        this->serviceListenerMap[serviceName] = vec;
    }
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#ServiceListenerInfo] Left." );
    return vec;
}

template<class ThreadingModel>
void IRegistryImpl<ThreadingModel>::addServiceListener( const std::string& bundleName, ServiceListenerInfoPtr listenerInfo )
{
    // !!! synchronized access !!!
    typename ThreadingModel::Lock lock;

    std::string serviceName = listenerInfo->getServiceName();
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#addServiceListener] Called, bundle name: %1, service name: %2", bundleName, serviceName );

    BundleInfoBase* bundleInfo = this->getBundleInfo( bundleName );

    if ( bundleInfo == 0 )
    {
        logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#addServiceListener] Bundle info is null, can not add service listener." );
    }
    else
    {
        logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#addServiceListener] Add service listener to service listener std::vector." );
        std::vector<ServiceListenerInfoPtr>* vec = this->getServiceListenerInfoVector( serviceName );
        vec->push_back( listenerInfo );
        logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#addServiceListener] Add service listener to bundle info." );
        bundleInfo->addRegisteredListener( listenerInfo );
    }

    std::vector<ServiceInfoPtr>* serviceVec = this->getServiceInfo( serviceName );

    this->notifyListenerAboutRegisteredService( bundleName, serviceVec, listenerInfo, serviceName );
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#addServiceListener] Left." );
}

template<class ThreadingModel>
void IRegistryImpl<ThreadingModel>::removeServiceListener( const std::string& bundleName, ServiceListenerInfoPtr serviceListener )
{
    // !!! synchronized access !!!
    typename ThreadingModel::Lock lock;

    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#removeServiceListener] Called, bundle name: %1", bundleName );
    this->removeFromServiceListenerInfoVector( bundleName, serviceListener );
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#removeServiceListener] Left" );
}

template<class ThreadingModel>
void IRegistryImpl<ThreadingModel>::removeFromServiceListenerInfoVector( const std::string& bundleName, ServiceListenerInfoPtr serviceListener )
{
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#removeFromServiceListenerInfoVector] Called." );
    std::map<std::string, std::vector<ServiceListenerInfoPtr>* >::iterator iter;
    for ( iter = this->serviceListenerMap.begin(); iter != this->serviceListenerMap.end(); iter++ )
    {
        std::vector<ServiceListenerInfoPtr>* vec = iter->second;
        std::vector<ServiceListenerInfoPtr>::iterator vecIterator;
        for ( vecIterator = vec->begin(); vecIterator != vec->end(); vecIterator++ )
        {
            if ( this->areServiceListenerObjectsEqual( (*vecIterator), serviceListener ) )
            {
                BundleInfoBase* bundleInfo  = this->getBundleInfo( bundleName );
                if ( bundleInfo == 0 )
                {
                    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#removeFromServiceListenerInfoVector] BundleInfo object does not exist (null), do nothing." );
                }
                else
                {
                    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#removeFromServiceListenerInfoVector] BundleInfo object exists, remove from std::vector." );
                    bundleInfo->removeUsedService( (*vecIterator)->getServiceName() );
                    bundleInfo->removeRegisteredListener( (*vecIterator) );
                }

                logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#removeFromServiceListenerInfoVector] Removed." );
                vec->erase( vecIterator );
                break;
            }
        }
    }
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#removeFromServiceListenerInfoVector] Left." );
}

template<class ThreadingModel>
bool IRegistryImpl<ThreadingModel>::areServiceListenerObjectsEqual( ServiceListenerInfoPtr info1, ServiceListenerInfoPtr info2 )
{
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#areServiceListenerObjectsEqual] Called, info1: %1, info2: %2", info1->toString(), info2->toString() );
    if ( info1->getServiceListenerObj() == info2->getServiceListenerObj() )
    {
        logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#areServiceListenerObjectsEqual] Objects are equal." );
        return true;
    }
    else
    {
        logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#areServiceListenerObjectsEqual] Objects are NOT equal." );
        return false;
    }
}

template<class ThreadingModel>
bool IRegistryImpl<ThreadingModel>::callServiceListenerObject( ServiceListenerInfoPtr info, ServiceInfoPtr serviceInfo, const ServiceEvent::EventType& eventType )
{
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#callServiceListenerObject] Called, serviceInfo: %1",
                serviceInfo->toString() );
    ServiceReference serviceRef( serviceInfo->getServiceName(), serviceInfo->getProperties(), serviceInfo->getService() );
    ServiceEvent servEvent( eventType, serviceRef );
    logger.log( Logger::LOG_DEBUG, "[IRegistryImpl#callServiceListenerObject] Called, listenerInfo: %1, event: %2",
                info->toString(), servEvent.toString() );
    bool b;
    b =  info->getServiceListenerObj()->serviceChanged( servEvent );
    return b;
}

