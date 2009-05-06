/**
 * @file test/util/t_ClockTimer.cpp
 *
 * Created      <2009-05-06 10:03:20 Ian Yang>
 * Last Updated <2009-05-06 10:20:03 Ian Yang>
 */
#include <boost/test/minimal.hpp>

#include <util/ClockTimer.h>
#include <boost/timer.hpp>

using namespace izenelib::util;

namespace { // {anonymous}

void mySleep(int seconds)
{
    boost::timer t;
    while (static_cast<int>(t.elapsed()) < seconds)
    {
        ; // do nothing but wait
    }
}

} // namespace {anonymous}

int test_main(int, char* [])
{
    ClockTimer t;

    mySleep(5);

    BOOST_CHECK(t.elapsed() >= 5); // wall clock > cpu time

    double e1 = t.elapsed();

    mySleep(1);

    BOOST_CHECK(e1 < t.elapsed()); // time keeps going

    t.restart();
    BOOST_CHECK(t.elapsed() < 5);

    return 0;
}
