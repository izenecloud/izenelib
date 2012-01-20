#include <util/criterion.h>

#include <boost/test/unit_test.hpp>

using namespace std;

int fib(int n)
{
  if (n<=1) return n;
  return fib(n-1) + fib(n-2);
}

BOOST_AUTO_TEST_SUITE( t_criterion )

BOOST_AUTO_TEST_CASE(main)
{
    bgroup("fib") {
        bench("10")
        fib(10);
        //bench("35")
        //fib(35);
        //bench("37")
        //fib(37);
    }
}

BOOST_AUTO_TEST_SUITE_END()

