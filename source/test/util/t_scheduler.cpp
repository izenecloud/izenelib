#include <util/scheduler.h>

#include <boost/test/unit_test.hpp>
#include <boost/timer.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <string>
#include <iostream>

using namespace izenelib::util;
using namespace std;

static int g_counter1 = 0;
static boost::mutex g_counter_mutex1;
void TestFuncOk1() {
  boost::mutex::scoped_lock l(g_counter_mutex1);
  ++g_counter1;
}

static int g_counter2 = 0;
static boost::mutex g_counter_mutex2;
void TestFuncOk2() {
  boost::mutex::scoped_lock l(g_counter_mutex2);
  ++g_counter2;
}

BOOST_AUTO_TEST_CASE(Scheduler_test)
{
    const string kTestJob1 = "Test1";
    const string kTestJob2 = "Test2";
    boost::function<void (void)> task1 = boost::bind(&TestFuncOk1);
    boost::function<void (void)> task2 = boost::bind(&TestFuncOk2);
  
    {
      g_counter1 = 0;
      Scheduler::addJob(kTestJob1, 1000, 1000, task1);
 
      boost::thread::sleep(boost::get_system_time() + boost::posix_time::milliseconds(3100));
      BOOST_CHECK(3==g_counter1);
      Scheduler::removeJob(kTestJob1);
    }
    {
      g_counter1 = 0;
      Scheduler::addJob(kTestJob1, 1000, 1000, task1);

      boost::thread::sleep(boost::get_system_time() + boost::posix_time::milliseconds(3100));
      BOOST_CHECK(3==g_counter1);

      g_counter2 = 0;
      Scheduler::addJob(kTestJob2, 1000, 1000, task2);

      boost::thread::sleep(boost::get_system_time() + boost::posix_time::milliseconds(3100));
	
      BOOST_CHECK(6==g_counter1);
      BOOST_CHECK(3==g_counter2);
      Scheduler::removeJob(kTestJob1);

      boost::thread::sleep(boost::get_system_time() + boost::posix_time::milliseconds(3100));
      BOOST_CHECK(6==g_counter1);
      BOOST_CHECK(6==g_counter2);
    }
    Scheduler::removeJob(kTestJob2);

    boost::thread::sleep(boost::get_system_time() + boost::posix_time::milliseconds(3100));
    BOOST_CHECK(6==g_counter1);
    BOOST_CHECK(6==g_counter2);
}

