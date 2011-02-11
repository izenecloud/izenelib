#ifndef SINGLE_THREADED_H
#define SINGLE_THREADED_H

#include <iostream>

using namespace std;

namespace izenelib
{
namespace osgi
{

/**
 * The <code>SingleThreaded</code> class is an default implementation
 * of the threading policy which can be defined when a new
 * {@link izenelib::osgi::Launcher} object is created.<br>
 * When the <code>SingleThreaded</code> class is used the calls made to
 * the registry class for example are not thread safe.
 *
 * @author magr74
 */
class SingleThreaded
{

public:

    /**
     * Each method of the SOF registry uses the <code>Lock</code> class.
     * Before executing a registry ({@link izenelib::osgi::IRegistry}) method
     * a local <code>Lock</code> instance is created which is automatically destroyed
     * when method call is left.<br>
     * That means that each registry method call can be made thread safe by implementing
     * a locking mechanism in the constructor and destructor of the <code>Lock</code> class.
     * Since the <code>SingleThreaded</code> class here does not any locking the constructor and
     * destructor implementation does not contain any code.
     */
    class Lock
    {
    public:

        /**
         * Creates instances of class <code>Lock</code>.
         */
        Lock()
        {
            // do nothing
        }

        /**
         * Destroys the object.
         */
        virtual ~Lock()
        {
            // do nothing
        }
    };
};

}
}
#endif
