#ifndef UTIL_CLOCK_TIMER_H
#define UTIL_CLOCK_TIMER_H
/**
 * @file util/ClockTimer.h
 * @author Ian Yang
 * @date Created <2009-05-06 09:34:37>
 * @date Updated <2010-06-25 15:06:09>
 * @brief Timer using wall clock instead of CPU ticks.
 *
 * In some performance measurement, the total elapsed time is more important
 * than just process or CPU time. @p izenelib::util::ClockTimer is such a tool
 * recording the total elapsed time.
 *
 * Because it use the Boost Date Time library, you may need to link that
 * library.
 */
#include <sys/time.h>

namespace izenelib {
namespace util {

/**
 * @brief A timer object similar to @p boost::timer but measures total elapsed
 * time instead of CPU time.
 *
 * This class measures wall clock rather than CPU time. Make your environment
 * clean because other processes can occupy the CPU, and the time is still counted.
 *
 * If CPU time measurement is intended, please use @p boost::timer in file
 * "boost/timer.hpp"
 */
class ClockTimer
{
public:
    /**
     * @brief remembers start time during construction
     */
    ClockTimer()
    : start_()
    {
        gettimeofday(&start_, 0);
    }

    /**
     * @brief resets start time to current time.
     */
    void restart()
    {
        gettimeofday(&start_, 0);
    }

    /**
     * @brief returns elapsed seconds since remembered start time.
     *
     * @return elapsed time in seconds, some platform can returns fraction
     * representing more precise time.
     */
    double elapsed() const
    {
        timeval now;
        gettimeofday(&now, 0);

        double seconds = now.tv_sec - start_.tv_sec;
        seconds += (now.tv_usec - start_.tv_usec) / 1000000.0;

        return seconds;
    }

private:
    timeval start_; /**< remembers start time */
};

}} // namespace izenelib::util

#endif // UTIL_CLOCK_TIMER_H
