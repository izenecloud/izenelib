///
///  @file  Semaphore.h
///  @date  2009-3-15
///  @author  Wei Cao
///  @brief I implement a light-weight semaphore in this file based on a mutex,
///			a conditional variable and a counter , It is much faster than
///			interprocess_semaphore supplied by boost::interprocess in boost-1.36

#ifndef _MESSAGE_FRAMEWORK_SEMAPHORE_H_
#define _MESSAGE_FRAMEWORK_SEMAPHORE_H_

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

namespace messageframework
{

	class light_semaphore : private boost::noncopyable{
	public:

    light_semaphore(int orig)
    {
		count_ = orig;
    }

    void post()
    {
		{
			boost::mutex::scoped_lock lock(mutex_);
			count_ ++;
		}
		cond_.notify_all();
    }

    void wait()
    {
		boost::mutex::scoped_lock lock(mutex_);
		if(count_ > 0)
			count_ --;
		else {
			while(count_ <=0 )
				cond_.wait(lock);
			count_ --;
		}
    }

    bool timed_wait(const boost::system_time& abs)
    {
		boost::mutex::scoped_lock lock(mutex_);
		if(count_ > 0)
			count_ --;
		else {
			while(count_ <=0 ) {
				if(!cond_.timed_wait(lock, abs))
				return false;
			}
			count_ --;
		}
		return true;
    }

	private:

		int count_;
		boost::mutex mutex_;
		boost::condition_variable cond_;
  };

  typedef light_semaphore semaphore_type;

#if 0
	#include <boost/interprocess/sync/interprocess_semaphore.hpp>
	typedef boost::interprocess::interprocess_semaphore semaphore_type;
#endif

}

#endif
