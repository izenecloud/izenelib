#include <util/timer.h>

#include <boost/date_time/posix_time/posix_time.hpp>

namespace izenelib{
namespace util{

class TimerEvent
{
public:
    TimerEvent() {}
    virtual ~TimerEvent() {}

    // Wait an event. Can set a maximum timeout value.
    // Return true if the event object receives a notification event from
    // different thread.
    bool wait(int msec)
    {
        boost::mutex::scoped_lock lock(mutex_);
        if (msec >= 0)
            return cond_.timed_wait(lock,boost::get_system_time() + boost::posix_time::milliseconds(msec));

        cond_.wait(lock);
        return true;
    }

    // Raise a notification event
    void notify()
    {
        cond_.notify_all();
    }

private:
    boost::condition_variable cond_;
    boost::mutex mutex_;
};

class TimerThread
{
public:
    TimerThread(uint32_t due_time, uint32_t interval,
                Timer *timer, TimerEvent *event)
            : due_time_(due_time),
            interval_(interval),
            timer_(timer),
            event_(event)
    {
        thread_.reset(new boost::thread(boost::bind(&TimerThread::run, this)));
    }

    virtual ~TimerThread() {}

    void join()
    {
        if (boost::this_thread::get_id() == thread_->get_id())
            return;
        thread_->join();
    }

    virtual void run()
    {
        if (event_->wait(due_time_))
        {
            return;
        }
        timer_->timerCallback();

        if (interval_ == 0)
        {
            return;
        }
        else
        {
            while (true)
            {
                if (event_->wait(interval_))
                {
                    return;
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
    TimerEvent *event_;
};

bool Timer::start(uint32_t due_time, uint32_t interval)
{
    if (timer_thread_.get() != NULL)
    {
        stop();
    }
    event_.reset(new TimerEvent);
    timer_thread_.reset(new TimerThread(due_time, interval, this, event_.get()));
    return true;
}

void Timer::stop()
{
    if (timer_thread_.get() == NULL)
    {
        return;
    }
    boost::mutex::scoped_lock l(mutex_);
    event_->notify();
    timer_thread_->join();
    timer_thread_.reset(NULL);
    event_.reset(NULL);
}

Timer::Timer() {}

Timer::~Timer()
{
    stop();
}

}
}
