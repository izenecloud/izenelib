#ifndef SCOPE_GUARD_H
#define SCOPE_GUARD_H

namespace izenelib
{
namespace osgi
{

/**
 * Takes responsibility of deleting an object when <code>ScopeGuard</code>
 * object goes out of scope.
 */
template<typename T>
class ScopeGuard
{
private:

    /**
     * The object which is deleted.
     */
    T* guardedObj;

public:

    /**
     * Creates instances of class <code>ScopeGuard</code>.
     *
     * @param guardedObject
     *         The object which is deleted if guard goes out
     *         of scope.
     */
    ScopeGuard( T* guardedObject );

    /**
     * Deletes the pointer to the guarded object.
     */
    virtual ~ScopeGuard();
};

template<typename T>
ScopeGuard<T>::ScopeGuard( T* guardedObject ) : guardedObj( guardedObject )
{
}

template<typename T>
ScopeGuard<T>::~ScopeGuard()
{
    delete this->guardedObj;
}

}
}
#endif
