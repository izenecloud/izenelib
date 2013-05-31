#ifndef IZENELIB_UTIL_READFAVORLOCK_H_
#define IZENELIB_UTIL_READFAVORLOCK_H_

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>
#include <3rdparty/folly/Likely.h>

namespace izenelib{
namespace util{

template<int FAVOR=10, class MutexType = boost::shared_mutex>
class ReadFavorLock : public MutexType
{

public:
    void lock() {
        //int lock_count=0;
        //while(!lock.try_lock()) 
        //{
            //lock_count++;
            //if(lock_count==10)
            //{
                //lock.lock();
                //break;
            //}
            //struct timespec ts;
            //ts.tv_sec = 0;
            //ts.tv_nsec = 10000;
            //int ret = nanosleep(&ts, NULL);
            //if(ret<0)
            //{
                //lock.lock();
                //break;
            //}
        //}
        int count = 0;
        while (!LIKELY(MutexType::try_lock())) {
            sched_yield();
            if (++count == FAVOR) 
            {
                MutexType::lock();
                break;
            }
        }
    }
//private:
    //MutexType mutex_;
};
}
}

#endif

