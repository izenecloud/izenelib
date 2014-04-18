/*
 * =====================================================================================
 *
 *       Filename:  condition.h
 *
 *    Description: pthread condition
 *
 *        Version:  1.0
 *        Created:  05/03/2011 03:56:51 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Kevin Hu (), kevin.hu@b5m.com
 *        Company:  iZeneSoft.com
 *
 * =====================================================================================
 */
#ifndef _IZENELIB_THREAD_CONDITION_H_
#define _IZENELIB_THREAD_CONDITION_H_

#include <types.h>
#include <vector>
#include <assert.h>
#include <iostream>
#include <pthread.h>
#include <sys/time.h>

#include "util/kthread/mutex.h"
namespace izenelib
{

class Condition
{
    Mutex* _mutex;
    pthread_cond_t _pcond;

private:
    Condition(const Condition& ) {}
    const Condition& operator= (const Condition&)
    {
        return *this;
    }

public:
    explicit Condition(Mutex* mutex = NULL)
        :_mutex(mutex)
    {
        pthread_cond_init(&_pcond, NULL);
    }

    ~Condition()
    {
        pthread_cond_destroy(&_pcond);
    }

    void set_mutex(Mutex* mutex)
    {
        _mutex = mutex;
    }

    void wait()
    {
        pthread_cond_wait(&_pcond, _mutex->pthread_mutex());
    }

    void wait(uint32_t sec)
    {
        struct timeval tv;
        struct timespec toWait;
        gettimeofday(&tv, NULL);

        toWait.tv_sec = tv.tv_sec + sec;
        toWait.tv_nsec = tv.tv_usec*1000;
        pthread_cond_timedwait(&_pcond, _mutex->pthread_mutex(), &toWait);
    }

    void notify()
    {
        pthread_cond_signal(&_pcond);
    }

    void notify_all()
    {
        pthread_cond_broadcast(&_pcond);
    }
};

}
#endif

