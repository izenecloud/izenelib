#include <util/osgi/BundleInfoBase.h>

#include <sstream>

using namespace std;

using namespace izenelib::osgi;

Logger& BundleInfoBase::logger_ = LoggerFactory::getLogger( "Framework" );

BundleInfoBase::BundleInfoBase( const string& bdleName, bool isSOFBundle, IBundleContext::ConstPtr bundleCtxt ) 
    :bundleName_(bdleName), 
     bundleContext_(bundleCtxt),
     isFwBundle_(isSOFBundle)
{
    logger_.log( Logger::LOG_DEBUG, "[BundleInfoBase#ctor] Called." );
}

BundleInfoBase::~BundleInfoBase()
{
    logger_.log( Logger::LOG_DEBUG, "[BundleInfoBase#destructor] Called." );
}

string BundleInfoBase::getBundleName() const
{
    return this->bundleName_;
}

IBundleContext::ConstPtr BundleInfoBase::getBundleContext()
{
    return this->bundleContext_;
}

void BundleInfoBase::addRegisteredService( ServiceInfoPtr serviceInfo )
{
    logger_.log( Logger::LOG_DEBUG, "[BundleInfoBase#addRegisteredService] Called." );
    this->registeredServices_.push_back( serviceInfo );
}

void BundleInfoBase::removeDeregisteredService( ServiceInfoPtr serviceInfo )
{
    logger_.log( Logger::LOG_DEBUG, "[BundleInfoBase#removeDeregisteredService] Called, service info: %1", serviceInfo->toString() );

    logger_.log( Logger::LOG_DEBUG, "[BundleInfoBase#removeDeregisteredService] Iterate over vector of registered services." );
    vector<ServiceInfoPtr>::iterator iter;
    for ( iter = this->registeredServices_.begin(); iter != this->registeredServices_.end(); ++iter )
    {
        if ( (*iter)->equals( (*((*iter).GetRawPointer() ) ), (*serviceInfo.GetRawPointer() ) ) )
        {
            logger_.log( Logger::LOG_DEBUG, "[BundleInfoBase#removeDeregisteredService] Service found." );
            //logger_.log( Logger::LOG_DEBUG, "[BundleInfoBase#removeDeregisteredService] Delete service object." );
            // TODO: no longer necessary due to usage of smart pointer
            //delete (*iter);
            logger_.log( Logger::LOG_DEBUG, "[BundleInfoBase#removeDeregisteredService] Service object deleted." );
            logger_.log( Logger::LOG_DEBUG, "[BundleInfoBase#removeDeregisteredService] Remove element from vector of registered services." );
            logger_.log( Logger::LOG_DEBUG, "[BundleInfoBase#removeDeregisteredService] Counter: %1", (( int ) (*iter).GetCount()));
            iter = this->registeredServices_.erase( iter );

            // TODO: Why breaking here?
            break;
        }
    }
}

void BundleInfoBase::removeUsedService( ServiceInfoPtr serviceInfo )
{
    logger_.log( Logger::LOG_DEBUG, "[BundleInfoBase#removeUsedService] Called, service info: %1", serviceInfo->toString() );

    logger_.log( Logger::LOG_DEBUG, "[BundleInfoBase#removeUsedService] Iterate over vector of used services." );

    vector<ServiceInfoPtr>::iterator iter;
    for ( iter = this->usedServices_.begin(); iter != this->usedServices_.end(); ++iter )
    {
        if ( (* ((*iter).GetRawPointer()) ) == (*(serviceInfo.GetRawPointer() ) ) )
        {
            logger_.log( Logger::LOG_DEBUG, "[BundleInfoBase#removeUsedService] Service found." );
            logger_.log( Logger::LOG_DEBUG, "[BundleInfoBase#removeUsedService] Remove element from vector of used services." );

            // TODO: Why not deleting object like in 'removeDeregisteredService'?
            iter = this->usedServices_.erase( iter );

            // TODO: Why breaking here?
            break;
        }
    }
}

void BundleInfoBase::removeAllUsedServices()
{
    logger_.log( Logger::LOG_DEBUG, "[BundleInfoBase#removeAllUsedServices] Called." );
    this->usedServices_.clear();
}

void BundleInfoBase::removeUsedService( const string& serviceName )
{
    logger_.log( Logger::LOG_DEBUG, "[BundleInfoBase#removeUsedService] Called, service name: %1", serviceName );
    logger_.log( Logger::LOG_DEBUG, "[BundleInfoBase#removeUsedService] Iterate over vector of used services." );

    vector<ServiceInfoPtr>::iterator iter;
    for ( iter = this->usedServices_.begin(); iter != this->usedServices_.end(); ++iter )
    {
        if ( (*iter)->getServiceName() == serviceName )
        {
            logger_.log( Logger::LOG_DEBUG, "[BundleInfoBase#removeUsedService] Service found." );
            logger_.log( Logger::LOG_DEBUG, "[BundleInfoBase#removeUsedService] Remove element from vector of used services." );

            iter = this->usedServices_.erase( iter );
            break;
        }
    }
}

void BundleInfoBase::addUsedService( ServiceInfoPtr serviceInfo )
{
    logger_.log( Logger::LOG_DEBUG, "[BundleInfoBase#addUsedService] Called, service info: %1", serviceInfo->toString() );

    logger_.log( Logger::LOG_DEBUG, "[BundleInfoBase#addUsedService] Check whether service info is already cached in vector." );

    vector<ServiceInfoPtr>::iterator iter;
    for ( iter = this->usedServices_.begin(); iter != this->usedServices_.end(); ++iter )
    {
        if ( (*((*iter).GetRawPointer()) ) == (*(serviceInfo.GetRawPointer())) )
        {
            logger_.log( Logger::LOG_DEBUG, "[BundleInfoBase#addUsedService] Service already cached, do not put it once again!" );
            return;
        }
    }
    logger_.log( Logger::LOG_DEBUG, "[BundleInfoBase#addUsedService] Put service info into vector." );
    this->usedServices_.push_back( serviceInfo );
}

void BundleInfoBase::addRegisteredListener( ServiceListenerInfoPtr listenerInfo )
{
    logger_.log( Logger::LOG_DEBUG, "[BundleInfoBase#addRegisteredListener] Called, service listener info: %1", listenerInfo->toString() );
    this->registeredListeners_.push_back( listenerInfo );
}

void BundleInfoBase::removeRegisteredListener( ServiceListenerInfoPtr listenerInfo )
{
    logger_.log( Logger::LOG_DEBUG, "[BundleInfoBase#removeRegisteredListener] Called, service listener info: %1",
                listenerInfo->toString() );
    logger_.log( Logger::LOG_DEBUG, "[BundleInfoBase#removeRegisteredListener] Iterate over vector of registered listeners." );

    vector<ServiceListenerInfoPtr>::iterator iter;
    for ( iter = this->registeredListeners_.begin(); iter != this->registeredListeners_.end(); ++iter )
    {
        if ( (*iter).GetRawPointer()->equals( (*(listenerInfo.GetRawPointer())), (*((*iter ).GetRawPointer())) )   )
        {
            logger_.log( Logger::LOG_DEBUG, "[BundleInfoBase#removeRegisteredListener] Listener found." );
            logger_.log( Logger::LOG_DEBUG, "[BundleInfoBase#removeRegisteredListener] Remove element from vector of registered listeners." );

            this->registeredListeners_.erase( iter );
            return;
        }
    }
}

bool BundleInfoBase::isFrameworkBundle() const
{
    return this->isFwBundle_;
}

string BundleInfoBase::toString() const
{
    ostringstream stream;
    stream << "*** Bundle: " << this->bundleName_ << " ****" << endl;

    stream << "*** Registered services ***" << endl;
    vector<ServiceInfoPtr>::const_iterator iter;

    for ( iter = registeredServices_.begin(); iter != registeredServices_.end(); ++iter )
    {
        stream << "  ->" << (*iter)->toString() << endl;
    }

    stream << "*** Services in use ***" << endl;
    for ( iter = usedServices_.begin(); iter != usedServices_.end(); ++iter )
    {
        stream << "  ->" << (*iter)->toString() << endl;
    }

    stream << "*** Registered service listener ***" << endl;
    vector<ServiceListenerInfoPtr>::const_iterator listenerIter;

    for ( listenerIter = registeredListeners_.begin(); listenerIter != registeredListeners_.end(); ++listenerIter )
    {
        stream << "  ->" << (*listenerIter)->toString() << endl;
    }
    stream << endl;
    return stream.str();
}

vector<ServiceInfoPtr> BundleInfoBase::getRegisteredServices() const
{
    return this->registeredServices_;
}

vector<ServiceInfoPtr> BundleInfoBase::getUsedServices() const
{
    return this->usedServices_;
}

vector<ServiceListenerInfoPtr> BundleInfoBase::getRegisteredListeners() const
{
    return this->registeredListeners_;
}
