#include <boost/test/unit_test.hpp>

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

BOOST_AUTO_TEST_CASE(ClockTimer_test)
{
    ClockTimer t;

    mySleep(5);
    double e1 = t.elapsed();

    mySleep(1);

    BOOST_CHECK(e1 < t.elapsed()); // time keeps going

    t.restart();
    BOOST_CHECK(t.elapsed() < 5);
}
