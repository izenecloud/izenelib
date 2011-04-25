#ifndef IZENE_UTIL_TIMESTAMP_H
#define IZENE_UTIL_TIMESTAMP_H

#include <stdint.h>
#include <iostream>

namespace izenelib{namespace util{
//----------------------------------------------------------------------------
// Timestamp
//----------------------------------------------------------------------------
class Timestamp
{
    // Time measured in microseconds since 1970-01-01T00:00:00Z.
    int64_t usec;

    explicit Timestamp(int64_t usec) : usec(usec) {}

public:
    Timestamp() : usec(0) {}

    /// Set the timestamp from broken down parts.  The values passed
    /// in should correspond to printable values in the ISO8601
    /// representation.  The timezone parameter, gmtoff, should be
    /// given as the second offset from GMT to the input timezone.
    void set(int year, int month, int day,
             int hour, int min, int sec,
             int32_t usec, int32_t gmtoff);

    /// Set the timestamp from broken down parts in the local
    /// timezone.
    void setLocal(int year, int month, int day,
                  int hour, int min, int sec,
                  int32_t usec);

    /// Set the timestamp from broken down parts in the UTC timezone.
    void setUtc(int year, int month, int day,
                int hour, int min, int sec,
                int32_t usec);

    operator int64_t() const
    {
        return usec;
    }

    bool operator< (Timestamp const & o) const
    {
        return usec <  o.usec;
    }
    bool operator<=(Timestamp const & o) const
    {
        return usec <= o.usec;
    }
    bool operator> (Timestamp const & o) const
    {
        return usec >  o.usec;
    }
    bool operator>=(Timestamp const & o) const
    {
        return usec >= o.usec;
    }
    bool operator==(Timestamp const & o) const
    {
        return usec == o.usec;
    }
    bool operator!=(Timestamp const & o) const
    {
        return usec != o.usec;
    }

    Timestamp operator+(Timestamp const & o) const
    {
        return Timestamp(usec + o.usec);
    }
    Timestamp operator-(Timestamp const & o) const
    {
        return Timestamp(usec - o.usec);
    }
    Timestamp & operator+=(Timestamp const & o)
    {
        usec += o.usec;
        return *this;
    }
    Timestamp & operator-=(Timestamp const & o)
    {
        usec -= o.usec;
        return *this;
    }

    /// Get a Timestamp for the current time.
    static Timestamp now();

    /// Get a Timestamp for the given seconds-from-Epoch time.
    static Timestamp fromSeconds(double s)
    {
        return Timestamp(int64_t(s * 1e6));
    }

    /// Get a Timestamp for the given seconds-from-Epoch time.
    static Timestamp fromSeconds(int64_t s)
    {
        return Timestamp(s * 1000000);
    }

    /// Get a Timestamp for the given milliseconds-from-Epoch time.
    static Timestamp fromMilliseconds(double ms)
    {
        return Timestamp(int64_t(ms * 1e3));
    }

    /// Get a Timestamp for the given milliseconds-from-Epoch time.
    static Timestamp fromMilliseconds(int64_t ms)
    {
        return Timestamp(ms * 1000);
    }

    /// Get a Timestamp for the given microseconds-from-Epoch time.
    static Timestamp fromMicroseconds(double us)
    {
        return Timestamp(int64_t(us));
    }

    /// Get a Timestamp for the given microseconds-from-Epoch time.
    static Timestamp fromMicroseconds(int64_t us)
    {
        return Timestamp(us);
    }
};

}
}
#endif // IZENE_UTIL_TIMESTAMP_H
