#ifndef IZENE_TIMER_H_
#define IZENE_TIMER_H_

#include <types.h>

#include <boost/thread.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>

namespace izenelib{
namespace util{

class TimerThread;
class Timer
{
public:
    typedef boost::function<void(void)> TimerCBType;
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
    bool start(uint32_t due_time, uint32_t interval, const TimerCBType& cb);

    // stop timer:
    // If pending signal functions remain, stop() blocks the current thread
    // Be sure that signal handler never be blocked.
    void stop(bool wait = false);

    Timer();

    // Timer() call stop() internally, so it is blocking
    ~Timer();

private:
    boost::scoped_ptr<TimerThread> timer_thread_;
};

}
}
#endif
