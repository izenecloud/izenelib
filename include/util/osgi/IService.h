#ifndef ISERVICE_H
#define ISERVICE_H

namespace izenelib{namespace osgi{
/**
 * The <code>IService</code> class defines only a
 * marker interface (interface which does not provide any
 * methods).<br>
 * Each service which wants to be registered with the framework
 * has to implement this interface in order to be identified as
 * service object.
 *
 * @author magr74
 */
class IService
{
public:

    /**
     * Defines a constant pointer to a service object.
     */
    typedef IService* const ConstPtr;

};

}}
#endif

