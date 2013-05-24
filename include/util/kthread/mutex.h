/*
 * =====================================================================================
 *
 *       Filename:  mutex.h
 *
 *    Description:  pthread mutex
 *
 *        Version:  1.0
 *        Created:  05/03/2011 03:46:41 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Kevin Hu (), kevin.hu@b5m.com
 *        Company:  iZeneSoft.com
 *
 * =====================================================================================
 */

#ifndef _IZENELIB_THREAD_MUTEX_H_
#define _IZENELIB_THREAD_MUTEX_H_

#include <types.h>
#include <vector>
#include <assert.h>
#include <iostream>
#include <pthread.h>

namespace izenelib
{

class Mutex
{
    pthread_mutex_t _mutex;

private:
    Mutex(const Mutex&) {}
    const Mutex& operator = (const Mutex&)
    {
        return *this;
    }

public:
    Mutex()
    {
        pthread_mutex_init(&_mutex, NULL);
    }

    ~Mutex()
    {
        pthread_mutex_destroy(&_mutex);
    }

    void lock()
    {
        pthread_mutex_lock(&_mutex);
    }

    void unlock()
    {
        pthread_mutex_unlock(&_mutex);
    }

    pthread_mutex_t* pthread_mutex() /*  non-const */
    {
        return &_mutex;
    }
};

class ScopedLock
{
    Mutex* _mutex;

public:
    ScopedLock(Mutex* mutex)
        :_mutex(mutex)
    {
        _mutex->lock();
    }

    ~ScopedLock()
    {
        _mutex->unlock();
    }
};

}
#endif

