#include <boost/test/unit_test.hpp>
#include <net/mrlite/signaling_queue.h>
#include <net/mrlite/socket_communicator.h>
#include <net/mrlite/util.h>

#include <sstream>
#include <string>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <sys/wait.h>

using net::mrlite::SignalingQueue;
using net::mrlite::SocketCommunicator;

using namespace boost;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using namespace net::mrlite;

const int kNumMessage = 100;

BOOST_AUTO_TEST_SUITE( t_communicator )

BOOST_AUTO_TEST_CASE( SignalingQueueTest_AddRemove )
{
    SignalingQueue queue(5, 1);  // size:5, num_of_producer:1
    char buff[10];
    queue.Add("111", 3);
    queue.Add("22", 2);
    BOOST_CHECK_EQUAL(0, queue.Add("xxxx", 4, false));	// non-blocking add
    queue.Remove(buff, 3);
    BOOST_CHECK_EQUAL(string(buff, 3), string("111"));
    queue.Remove(buff, 2);
    BOOST_CHECK_EQUAL(string(buff, 2), string("22"));
    queue.Add("33333", 5);
    queue.Remove(buff, 5);
    BOOST_CHECK_EQUAL(string(buff, 5), string("33333"));
    BOOST_CHECK_EQUAL(0, queue.Remove(buff, 10, false));  // non-blocking remove
    BOOST_CHECK_EQUAL(queue.Add("666666", 6), -1);	// exceed buffer size
    queue.Add("55555", 5);
    BOOST_CHECK_EQUAL(queue.Remove(buff, 3), -1);  // message too long

}

BOOST_AUTO_TEST_CASE(SignalingQueueTest_EmptyAndNoMoreAdd)
{
    SignalingQueue queue(5, 2);  // size:5, num_of_producer:2
    char buff[10];
    BOOST_CHECK_EQUAL(queue.EmptyAndNoMoreAdd(), false);
    queue.Signal(1);
    queue.Signal(1);
    BOOST_CHECK_EQUAL(queue.EmptyAndNoMoreAdd(), false);
    queue.Signal(2);
    BOOST_CHECK_EQUAL(queue.EmptyAndNoMoreAdd(), true);
    BOOST_CHECK_EQUAL(queue.Remove(buff, 5), 0);
}


// this test forks 4 processes:
// 2 map workers and 2 reduce workers
BOOST_AUTO_TEST_CASE(CommunicatorTest)
{
    int pid;
    pid = fork();
    BOOST_CHECK_LE(0, pid);
    vector<string> reducers;
    SplitStringUsing(
        string("127.0.0.1:10103,127.0.0.1:10104"),
        ",",
        &reducers);

    if (pid > 0)    // parent: 2 reducers
    {
        pid = fork();
        BOOST_CHECK_LE(0, pid);
        if (pid > 0)    // reducer 0
        {
            SocketCommunicator r0;
            BOOST_CHECK(r0.Initialize(false,
                                      2,
                                      reducers,
                                      1024, 1024,
                                      0));
            string result;
            int message_count_0 = 0;
            int message_count_1 = 0;
            while (true)
            {
                int retval = r0.Receive(&result);
                BOOST_CHECK_LE(0, retval);
                if (0 == retval) break;
                if (result[1] == '0')
                {
                    BOOST_CHECK_EQUAL(string("m0r0"), result);
                    ++message_count_0;
                }
                else
                {
                    BOOST_CHECK_EQUAL(string("m11r0"), result);
                    ++message_count_1;
                }
            }
            BOOST_CHECK_EQUAL(kNumMessage, message_count_0);
            BOOST_CHECK_EQUAL(kNumMessage, message_count_1);
            BOOST_CHECK(r0.Finalize());
        }
        else    // reducer 1
        {
            sleep(1);
            SocketCommunicator r1;
            BOOST_CHECK(r1.Initialize(false,
                                      2,
                                      reducers,
                                      1024, 1024,
                                      1));
            string result;
            int message_count_0 = 0;
            int message_count_1 = 0;
            while (true)
            {
                int retval = r1.Receive(&result);
                BOOST_CHECK_LE(0, retval);
                if (0 == retval) break;
                if (result[1] == '0')
                {
                    BOOST_CHECK_EQUAL(string("m0r11"), result);
                    ++message_count_0;
                }
                else
                {
                    BOOST_CHECK_EQUAL(string("m11r11"), result);
                    ++message_count_1;
                }
            }
            BOOST_CHECK_EQUAL(kNumMessage, message_count_0);
            BOOST_CHECK_EQUAL(kNumMessage, message_count_1);
            BOOST_CHECK(r1.Finalize());
        }
    }
    else    // child: 2 mappers
    {
        sleep(2);
        pid = fork();
        BOOST_CHECK_LE(0, pid);
        if (pid > 0)    // mapper 0
        {
            SocketCommunicator m0;
            BOOST_CHECK(m0.Initialize(true,
                                      2,
                                      reducers,
                                      1024, 1024,
                                      0));
            for (int i = 0; i < kNumMessage; ++i)
            {
                BOOST_CHECK_LE(0, m0.Send(string("m0r0"), 0));
                BOOST_CHECK_LE(0, m0.Send(string("m0r11"), 1));
            }
            BOOST_CHECK(m0.Finalize());
        }
        else    // mapper 1
        {
            SocketCommunicator m1;
            BOOST_CHECK(m1.Initialize(true,
                                      2,
                                      reducers,
                                      1024, 1024,
                                      1));
            for (int i = 0; i < kNumMessage; ++i)
            {
                BOOST_CHECK_LE(0, m1.Send(string("m11r0"), 0));
                BOOST_CHECK_LE(0, m1.Send(string("m11r11"), 1));
            }
            BOOST_CHECK(m1.Finalize());
        }
    }
    wait(0);
}

BOOST_AUTO_TEST_SUITE_END()
