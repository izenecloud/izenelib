#include <time.h>
#include <sys/time.h>
#include <stdio.h> // snprintf
#include <stdlib.h> // getenv

#include <util/timestamp.h>

namespace izenelib{namespace util{

inline size_t fmt_gmtoff(char * buf, size_t bufsz, int32_t gmtoff,
                         bool extended)
{
    int s;
    char sign;
    if (gmtoff < 0)
    {
        s = -gmtoff;
        sign = '-';
    }
    else
    {
        s = gmtoff;
        sign = '+';
    }

    int m = s / 60;
    s -= m * 60;

    int h = (m / 60) % 24;
    m -= h * 60;

    int sz;
    if (extended)
    {
        if (s)
            sz = snprintf(buf, bufsz, "%c%02d:%02d:%02d", sign, h, m, s);
        else if (m)
            sz = snprintf(buf, bufsz, "%c%02d:%02d", sign, h, m);
        else
            sz = snprintf(buf, bufsz, "%c%02d", sign, h);
    }
    else
    {
        if (m)
            sz = snprintf(buf, bufsz, "%c%02d%02d", sign, h, m);
        else
            sz = snprintf(buf, bufsz, "%c%02d", sign, h);
    }

    if (sz > 0)
        return size_t(sz);
    else
        return 0;
}

void Timestamp::set(int year, int month, int day,
                    int hour, int min, int sec,
                    int32_t usec, int32_t gmtoff)
{
    struct tm t;

    t.tm_sec    = sec;
    t.tm_min    = min;
    t.tm_hour   = hour;
    t.tm_mday   = day;
    t.tm_mon    = month - 1;
    t.tm_year   = year - 1900;
    t.tm_isdst  = 0;

    char tzBuf[64] = "UTC";
    if (gmtoff)
        fmt_gmtoff(tzBuf+3, sizeof(tzBuf)-3, -gmtoff, true);

    // Race condition: mktime() uses the TZ environment variable.  We
    // set and restore here, but we have no way to guarantee another
    // thread won't change it before we manage to call mktime().
    {
        std::string oldTzBuf;
        char const * oldTz = 0;
        if (char const * tz = getenv("TZ"))
        {
            oldTzBuf = tz;
            oldTz = oldTzBuf.c_str();
        }

        setenv("TZ", tzBuf, 1);

        this->usec = mktime(&t);

        if (oldTz)
            setenv("TZ", oldTz, 1);
        else
            unsetenv("TZ");
    }
    // End race

    //this->usec  = this->usec * 1000000 + usec;
}

void Timestamp::setLocal(int year, int month, int day,
                         int hour, int min, int sec,
                         int32_t usec)
{
    struct tm t;

    t.tm_sec    = sec;
    t.tm_min    = min;
    t.tm_hour   = hour;
    t.tm_mday   = day;
    t.tm_mon    = month - 1;
    t.tm_year   = year - 1900;
    t.tm_isdst  = -1;

    // Race condition: mktime() uses whatever the current timezone
    // happens to be, which may have changed in another thread since
    // this method was called.
    this->usec  = mktime(&t);
    //this->usec  = this->usec * 1000000 + usec;
}

void Timestamp::setUtc(int year, int month, int day,
                       int hour, int min, int sec,
                       int32_t usec)
{
    set(year, month, day,
        hour, min, sec,
        usec, 0);
}


Timestamp Timestamp::now()
{
    struct timeval tv;
    gettimeofday(&tv, 0);

    //return Timestamp(int64_t(tv.tv_sec)  * 1000000 + int64_t(tv.tv_usec));
    return Timestamp(int64_t(tv.tv_sec)  * 1000000);
}

}}

