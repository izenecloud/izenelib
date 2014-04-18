#include <util/timer.h>

#include <boost/date_time/posix_time/posix_time.hpp>

namespace izenelib{
namespace util{

class TimerThread
{
public:
    TimerThread(uint32_t due_time, uint32_t interval,
                const Timer::TimerCBType& cb)
            : due_time_(due_time),
            interval_(interval),
            callback_(cb),
            stop_(false)
    {
        thread_.reset(new boost::thread(boost::bind(&TimerThread::run, this)));
    }

    ~TimerThread() 
    {
        stop(true);
    }

    void stop(bool wait = false)
    {
        {
            boost::mutex::scoped_lock lock(mutex_);
            stop_ = true;
            cond_.notify_all();
        }
        if (!wait)
            return;
        if (boost::this_thread::get_id() == thread_->get_id())
        {
            return;
        }
        try{
        thread_->join();
        }catch(const std::exception& e)
        {
            std::cerr << "join timer thread error: " << e.what() << std::endl; 
        }
    }

    void run()
    {
        {
            boost::mutex::scoped_lock lock(mutex_);
            cond_.timed_wait(lock,boost::get_system_time() + boost::posix_time::milliseconds(due_time_));
            if (stop_)
            {
                callback_ = NULL;
                return;
            }
        }

        callback_();

        if (interval_ == 0)
        {
            callback_ = NULL;
            return;
        }
        else
        {
            while (!stop_)
            {
                {
                    boost::mutex::scoped_lock lock(mutex_);
                    cond_.timed_wait(lock,boost::get_system_time() +
                        boost::posix_time::milliseconds(interval_));
                    if (stop_)
                    {
                        callback_ = NULL;
                        return;
                    }
                }
                
                callback_();
            }
        }
        callback_ = NULL;
    }

private:
    boost::scoped_ptr<boost::thread> thread_;
    uint32_t due_time_;
    uint32_t interval_;
    Timer::TimerCBType callback_;
    bool stop_;
    boost::condition_variable cond_;
    boost::mutex mutex_;
};

bool Timer::start(uint32_t due_time, uint32_t interval, const TimerCBType& cb)
{
    if (timer_thread_ || cb == NULL)
    {
        // can not start timer twice or without timer callback.
        return false;
    }
    timer_thread_.reset(new TimerThread(due_time, interval, cb));
    return true;
}

void Timer::stop(bool wait)
{
    if (!timer_thread_)
    {
        return;
    }
    timer_thread_->stop(wait);
}

Timer::Timer() {}

Timer::~Timer()
{
}

}
}
