#ifndef IZENE_TIMER_H_
#define IZENE_TIMER_H_

#include <types.h>

#include <boost/thread.hpp>
#include <boost/scoped_ptr.hpp>

namespace izenelib {
namespace util {

class TimerThread;
class UnnamedEvent;
class Timer {
public:
    // Start timer.
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

    // Stop timer:
    // If pending signal functions remain, Stop() blocks the current thread
    // Be sure that signal handler never be blocked.
    void stop();

    Timer();

    // Timer() call Stop() internally, so it is blocking
    virtual ~Timer();

    void timerCallback() {
        boost::mutex::scoped_lock l(mutex_);
        num_signaled_++;
        signaled();
    }

protected:
    // overwrite this function to implement
    // signal handler.
    virtual void signaled() {}

private:
    boost::mutex mutex_;
    boost::scoped_ptr<UnnamedEvent> event_;
    boost::scoped_ptr<TimerThread> timer_thread_;
    uint32_t num_signaled_;
};

}}
#endif
