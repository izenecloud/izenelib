#ifndef AM_CONCEPT_THREADMODEL_H
#define AM_CONCEPT_THREADMODEL_H

#include <boost/noncopyable.hpp>
#include <boost/thread/shared_mutex.hpp>

NS_IZENELIB_AM_BEGIN

class NullLock
{
public:
    inline int acquire_read_lock(){
        return 0;
    }

    inline int release_read_lock(){
        return 0;
    }

    inline int actuire_write_lock(){
        return 0;
    }

    inline int release_write_lock(){
        return 0;
    }
};

/* class ReadWriteLock : private boost::noncopyable */
/* { */
/* private: */
/*     boost::shared_mutex rwMutex_; */
/* public: */
/*     ReadWriteLock() */
/*     { */
/*     } */

/*     ~ReadWriteLock() */
/*     { */
/*     } */

/*     /\** */
/*      @ brief Attempts to get the read lock. */
/*     *\/ */
/*     inline int acquire_read_lock{ */
/*         rwMutex_.lock_shared(); */
/*         return 0 ; */
/*     } */
/*     /\** */
/*      @ brief Attempts to get the write lock. */
/*     *\/ */
/*     inline int acquire_write_lock{ */
/*         rwMutex_.lock(); */
/*         return 0 ; */
/*     } */
/*     /\** */
/*      @ brief Attempts to release the  read lock . */
/*     *\/ */
/*     inline int release_read_lock(){ */
/*         rwMutex_.unlockshared(); */
/*         return 0; */
/*     } */
/*     /\** */
/*      @ brief Attempts to release the write lock. */
/*     *\/ */
/*     inline int release_write_lock(){ */
/*         rwMutex_.unlock(); */
/*         return 0; */
/*     } */

/* }; */


NS_IZENELIB_AM_END

#endif //Endof AM_CONCEPT_THREADMODEL_H
