#include <util/timer.h>

#include <boost/date_time/posix_time/posix_time.hpp>

namespace izenelib{
namespace util{

class TimerThread
{
public:
    TimerThread(uint32_t due_time, uint32_t interval,
                Timer *timer)
            : due_time_(due_time),
            interval_(interval),
            timer_(timer),
            stop_(false)
    {
        thread_.reset(new boost::thread(boost::bind(&TimerThread::run, this)));
    }

    ~TimerThread() {}

    void stop()
    {
        {
            boost::mutex::scoped_lock lock(mutex_);
            stop_ = true;
            cond_.notify_all();
        }
        if (boost::this_thread::get_id() == thread_->get_id())
        {
            return;
        }
        thread_->join();
    }

    void run()
    {
        {
            boost::mutex::scoped_lock lock(mutex_);
            cond_.timed_wait(lock,boost::get_system_time() + boost::posix_time::milliseconds(due_time_));
            if (stop_)
            {
                return;
            }
        }
        timer_->timerCallback();

        if (interval_ == 0)
        {
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
                        return;
                    }
                }
                
                timer_->timerCallback();
            }
        }
    }

private:
    boost::scoped_ptr<boost::thread> thread_;
    uint32_t due_time_;
    uint32_t interval_;
    Timer *timer_;
    bool stop_;
    boost::condition_variable cond_;
    boost::mutex mutex_;
};

bool Timer::start(uint32_t due_time, uint32_t interval)
{
    if (timer_thread_.get() != NULL)
    {
        stop();
    }
    timer_thread_.reset(new TimerThread(due_time, interval, this));
    return true;
}

void Timer::stop()
{
    if (timer_thread_.get() == NULL)
    {
        return;
    }
    timer_thread_->stop();
    timer_thread_.reset(NULL);
}

Timer::Timer() {}

Timer::~Timer()
{
    stop();
}

}
}
