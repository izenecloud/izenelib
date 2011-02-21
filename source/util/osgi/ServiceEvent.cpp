#include <util/osgi/ServiceEvent.h>

using namespace izenelib::osgi;

ServiceEvent::ServiceEvent( int eventType, const ServiceReference& ref ) : type_( eventType ), reference_( ref )
{
}

int ServiceEvent::getType() const
{
    return this->type_;
}

ServiceReference ServiceEvent::getReference() const
{
    return this->reference_;
}


string ServiceEvent::toString() const
{
    ostringstream eventStream;
    eventStream << "ServiceEvent={";
    eventStream << "type_=" << this->type_ << ", ";
    eventStream << this->reference_.toString();
    eventStream << "}";
    return eventStream.str();
}

