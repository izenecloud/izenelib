#ifndef CONCURRENT_LOCKS_H
#define CONCURRENT_LOCKS_H

#define CACHE_LINE 64
#define LOOP_SLESHOLD 2048
#include <stdint.h>
#include <time.h>
#include <pthread.h>

#include "atomic.h"

namespace concurrent{

namespace detail
{
static void micro_sleep(const int t)
{
    struct ::timespec req;
    req.tv_sec = 0;
    req.tv_nsec = t * 1000;
    ::nanosleep(&req, NULL);
}


class spin_lock
{
public:
    spin_lock() {};
    void lock()
    {
        int count = 0;
        while (1)
        {
            if (*static_cast<volatile uint32_t*>(&flag.lock) == 1) // already locked !
            {
                if (count >= LOOP_SLESHOLD)
                {
                    count = 0;
                    micro_sleep(21);
                }
                else
                {
                    count++;
                    pthread_yield();
                }
                continue;
            }

            if (!__sync_lock_test_and_set (static_cast<volatile uint32_t*>(&flag.lock), 1))
                //if(__sync_bool_compare_and_swap(static_cast<uint32_t*>(&flag.lock),
                //0, 1))
                //if(flag.lock == 0)
            {
                detail::reorder_partition();
                return;
            }
        }
    }
    void unlock()
    {
        //__sync_lock_test_and_set(static_cast<uint32_t*>(&flag.lock), 0);
        __sync_lock_release(static_cast<volatile uint32_t*>(&flag.lock), 0);
        detail::reorder_partition();
        //flag.lock = 0;
    }
private:
    spin_lock(const spin_lock&);
    spin_lock& operator=(const spin_lock&);
    union aligned_flag
    {
        volatile uint32_t lock;
        char padding[CACHE_LINE - sizeof(uint32_t)];
        aligned_flag():lock(0) {}
    } __attribute__((aligned(CACHE_LINE)));
    aligned_flag flag;
};


template <typename lk>
class scoped_lock
{
public:
    scoped_lock(lk& lock)
    {
        ptr = &lock;
        ptr->lock();
    };
    void unlock()
    {
        // you can't relock this object
        if (ptr)
        {
            ptr->unlock();
            ptr = NULL;
        }
    }
    ~scoped_lock()
    {
        if (ptr)
        {
            ptr->unlock();
        }
    }
private:
    scoped_lock();
    scoped_lock(const scoped_lock&);
    scoped_lock& operator=(const scoped_lock&);
    lk* ptr;
};
}

}
#endif

