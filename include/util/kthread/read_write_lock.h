/*
 * =====================================================================================
 *
 *       Filename:  read_write_lock.h
 *
 *    Description:  Read write lock
 *
 *        Version:  1.0
 *        Created:  2013年05月23日 14时27分01秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Kevin Hu (), kevin.hu@b5m.com
 *        Company:  B5M.com
 *
 * =====================================================================================
 */

#ifndef _IZENELIB_THREAD_RWLOCK_H_
#define _IZENELIB_THREAD_RWLOCK_H_

#include "types.h"
#include <vector>
#include <assert.h>
#include <iostream>
#include <pthread.h>

namespace izenelib
{
class ReadWriteLock
{
    pthread_rwlock_t rwlock_;

public:
    ReadWriteLock()
    {
        if(pthread_rwlock_init(&rwlock_, NULL) != 0)
            throw std::runtime_error("Can't initialize read & write lock.");
    }

    class Lock
    {
        pthread_rwlock_t *rwlock_;
    public:
        Lock(pthread_rwlock_t *rwlock)
            :rwlock_(rwlock)
        {
            IASSERT(rwlock_ != NULL);
        }

        ~Lock()
        {
            pthread_rwlock_unlock(rwlock_);
        }
    };

    pthread_rwlock_t* read()
    {
        pthread_rwlock_rdlock(&rwlock_);
        return &rwlock_;
    }

    pthread_rwlock_t* write()
    {
        pthread_rwlock_wrlock(&rwlock_);
        return &rwlock_;
    }
};

#define READ_LOCK(obj) izenelib::ReadWriteLock::Lock rrrrrrrrlock(obj.read());
#define WRITE_LOCK(obj) izenelib::ReadWriteLock::Lock wwwwwwwlock(obj.write());

}
#endif
