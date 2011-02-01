#ifndef OSGI_BASE_FACTORY_H
#define OSGI_BASE_FACTORY_H

namespace izenelib{namespace osgi{

/**
 * This template based class provides an interface
 * for creating instances of type <code>BaseT</code>.
 *
 * @author magr74
 */
template<typename BaseT>
class BaseFactory
{
public:
    virtual ~BaseFactory(){}
    /**
     * Creates instances of type <code>BaseT</code>.
     *
     * @return
     * An instance of type <code>BaseT</code>.
     */
    virtual BaseT* newInstance() = 0;
};

}}
#endif

