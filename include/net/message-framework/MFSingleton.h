/**
 *
 * @details
 * Code from Boost Cookbook. http://www.boostcookbook.com/
 */

#ifndef _MF_SINGLETON_H_
#define _MF_SINGLETON_H_

#include <boost/utility.hpp>
#include <boost/thread/once.hpp>
#include <boost/scoped_ptr.hpp>

// Warning: If T's constructor throws, instance() will return a null reference.

namespace messageframework
{

    template<class T>
    class MFSingleton : private boost::noncopyable
    {

        public:
            static T& instance()
            {
                boost::call_once(init, flag);
                return *t;
            }

            static void init() // never throws
            {
                t.reset(new T());
            }

        protected:
            ~MFSingleton() {}
            MFSingleton() {}

        private:
            static boost::scoped_ptr<T> t;
            static boost::once_flag flag;

    };

}//namespace

template<class T> boost::scoped_ptr<T> messageframework::MFSingleton<T>::t(0);
template<class T> boost::once_flag messageframework::MFSingleton<T>::flag = BOOST_ONCE_INIT;

#endif  //_MF_SINGLETON_H_
