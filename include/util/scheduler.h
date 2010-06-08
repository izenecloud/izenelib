
// Scheduler is a timer, call registered callback at a given interval.
// Scheduler has following features.
// usage:
// // start scheduled job
// Scheduler::AddJob("TimerName", 60*1000, 60*60*1000, 30*1000,callback);
// // stop job
// Scheduler::RemoveJob("TimerName");

#ifndef IZENE_UTIL_SCHEDULER_H_
#define IZENE_UTIL_SCHEDULER_H_

#include <types.h>

#include <string>

#include <boost/function.hpp>

namespace izenelib {
namespace util {
class Timer;

class Scheduler {
   public:
    // start scheduled job
    // return false if timer is already existed.
    // job will start after (delay_start + random_delay*rand()) msec
    static bool addJob(const std::string &name, uint32_t default_interval,
                       uint32_t delay_start, const boost::function<void (void)>& func);

    // stop scheduled job
    static bool removeJob(const std::string &name);

    // stop all jobs
    static void removeAllJobs();

    // should never be allocated.
    private:
    Scheduler();
    virtual ~Scheduler();
};

}}
#endif  // IZENE_UTIL_SCHEDULER_H_
