/***************************************************************************
 *  Copyright (C) 2002 Roman Dementiev <dementiev@mpi-sb.mpg.de>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See accompanying file LICENSE_1_0.txt or copy at
 *  http://www.boost.org/LICENSE_1_0.txt)
 **************************************************************************/

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/noncopyable.hpp>



class semaphore : private boost::noncopyable
{
    int v;
    boost::mutex mutex;
    boost::condition cond;

public:
    semaphore(int init_value = 1) : v(init_value)
    {
    }
    ~semaphore()
    {
    }
    // function increments the semaphore and signals any threads that
    // are blocked waiting a change in the semaphore
    int operator ++ (int)
    {
        boost::mutex::scoped_lock Lock(mutex);
        int res = ++v;
        Lock.unlock();
        cond.notify_one();
        return res;
    }
    // function decrements the semaphore and blocks if the semaphore is
    // <= 0 until another thread signals a change
    int operator -- (int)
    {
        boost::mutex::scoped_lock Lock(mutex);
        while (v <= 0)
            cond.wait(Lock);

        int res = --v;
        return res;
    }
    // function does NOT block but simply decrements the semaphore
    // should not be used instead of down -- only for programs where
    // multiple threads must up on a semaphore before another thread
    // can go down, i.e., allows programmer to set the semaphore to
    // a negative value prior to using it for synchronization.
    int decrement()
    {
        boost::mutex::scoped_lock Lock(mutex);
        return (--v);
    }
};

#endif

