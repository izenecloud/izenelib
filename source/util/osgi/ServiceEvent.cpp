#include <util/osgi/ServiceEvent.h>

using namespace izenelib::osgi;

ServiceEvent::ServiceEvent( int eventType, const ServiceReference& ref ) : type( eventType ), reference( ref )
{

}

int ServiceEvent::getType() const
{
    return this->type;
}

ServiceReference ServiceEvent::getReference() const
{
    return this->reference;
}


string ServiceEvent::toString() const
{
    ostringstream eventStream;
    eventStream << "ServiceEvent={";
    eventStream << "type=" << this->type << ", ";
    eventStream << this->reference.toString();
    eventStream << "}";
    return eventStream.str();
}

