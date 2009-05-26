/**
 * @file util/ClockTimer.h
 * @author Ian Yang
 * @date Created <2009-05-06 09:34:37>
 * @date Updated <2009-05-26 10:35:24>
 * @brief Timer using wall clock instead of CPU ticks.
 *
 * In some performance measurement, the total elapsed time is more important
 * than just process or CPU time. @p izenelib::util::ClockTimer is such a tool
 * recording the total elapsed time.
 *
 * Because it use the Boost Date Time library, you may need to link that
 * library.
 */
#ifndef UTIL_CLOCK_TIMER_H
#define UTIL_CLOCK_TIMER_H

#include <boost/date_time/posix_time/posix_time.hpp>

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
    : start_(boost::posix_time::microsec_clock::local_time())
    {}

    /**
     * @brief resets start time to current time.
     */
    void restart()
    {
        start_ = boost::posix_time::microsec_clock::local_time();
    }

    /**
     * @brief returns elapsed seconds since remembered start time.
     *
     * @return elapsed time in seconds, some platform can returns fraction
     * representing more precise time.
     */
    double elapsed() const
    {
        boost::posix_time::time_duration td =
            boost::posix_time::microsec_clock::local_time() -
            start_;

        return static_cast<double>(td.fractional_seconds()) /
            boost::posix_time::time_duration::ticks_per_second()
            + td.total_seconds();
    }

private:
    boost::posix_time::ptime start_; /**< remembers start time */
};

}} // namespace izenelib::util

#endif // UTIL_CLOCK_TIMER_H
