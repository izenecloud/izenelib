#ifndef _COMMON_POSIXTIMER_HPP_
#define _COMMON_POSIXTIMER_HPP_

#include <sys/time.h>

class POSIXTimer
{
public:

    POSIXTimer() : start_t(), end_t(), dt() {}

    void start()
    {
        gettimeofday( &start_t, 0 );
    }

    void stop()
    {
        gettimeofday( &end_t, 0 );

        dt += getMicroseconds(end_t) - getMicroseconds(start_t);
    }

    double getElapsedTimeInSeconds()
    {
        return dt * 1e-6;
    }

    void reset()
    {
        dt = 0;
    }

private:

    struct timeval start_t;

    struct timeval end_t;

    unsigned long long dt;

    unsigned long long getMicroseconds(const timeval& t)
    {
        return t.tv_sec * 1000000 + t.tv_usec;
    }

};



#endif
