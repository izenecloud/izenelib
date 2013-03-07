#ifndef IZENE_TIMER_H_
#define IZENE_TIMER_H_

#include <types.h>

#include <boost/thread.hpp>
#include <boost/scoped_ptr.hpp>

namespace izenelib{
namespace util{

class TimerThread;
class Timer
{
public:
    // start timer.
    // due_time:
    // The amount of time to elapse before the timer is to be set to the
    // signaled state for the first time, in milliseconds.
    // interval:
    // The period of the timer, in milliseconds. If |interval| is zero,
    // the timer is one-shot timer. If this parameter is greater than zero,
    // the timer is periodic.
    //
    // Either due_time and interval must be non 0.
    bool start(uint32_t due_time, uint32_t interval);

    // stop timer:
    // If pending signal functions remain, stop() blocks the current thread
    // Be sure that signal handler never be blocked.
    void stop();

    Timer();

    // Timer() call stop() internally, so it is blocking
    virtual ~Timer();

    void timerCallback()
    {
        signaled();
    }

protected:
    // overwrite this function to implement
    // signal handler.
    virtual void signaled() {}

private:
    boost::scoped_ptr<TimerThread> timer_thread_;
};

}
}
#endif
