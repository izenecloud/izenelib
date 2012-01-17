#include <boost/test/unit_test.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/random.hpp>
#include <boost/timer.hpp>

#include <iostream>
#include <string>
#include <stdint.h>
#include <limits.h>

#include <am/concurrent/skiplist.hpp>

using std::string;
using boost::shared_ptr;
using izenelib::am::concurrent::SkipList;

BOOST_AUTO_TEST_SUITE( t_concurrent_skiplist_suite )

BOOST_AUTO_TEST_CASE(insert_one)
{
    SkipList<int, string,4> dat(INT_MIN,INT_MAX);
    BOOST_CHECK(dat.add(2,"tmp"));
    std::cerr << "insert ok" << std::endl;
}

BOOST_AUTO_TEST_CASE(insertone_contains)
{
    SkipList<int, string,4> dat(INT_MIN,INT_MAX);
    BOOST_CHECK(dat.add(2,"tmp"));
    BOOST_CHECK(!dat.contains(1));
    BOOST_CHECK(dat.contains(2));
    BOOST_CHECK(!dat.contains(3));
    dat.dump();
}

BOOST_AUTO_TEST_CASE(insert_two)
{
    SkipList<int, string,4> dat(INT_MIN,INT_MAX);
    BOOST_CHECK(dat.add(2,"v2"));
    BOOST_CHECK(dat.add(3,"v3"));
    dat.dump();
}

BOOST_AUTO_TEST_CASE(insert_two_contains)
{
    SkipList<int, string,4> dat(INT_MIN,INT_MAX);
    BOOST_CHECK(dat.add(2,"tmp"));
    BOOST_CHECK(dat.add(3,"v3"));
    BOOST_CHECK(dat.contains(2));
    BOOST_CHECK(dat.contains(3));
    dat.dump();
}

BOOST_AUTO_TEST_CASE(insert_collide)
{
    SkipList<int, string,4> dat(INT_MIN,INT_MAX);
    BOOST_CHECK(dat.add(2,"tmp"));
    BOOST_CHECK(!dat.add(2,"tmp"));
    BOOST_CHECK(dat.contains(2));
}

BOOST_AUTO_TEST_CASE(remove_one)
{
    SkipList<int, string,4> dat(INT_MIN,INT_MAX);
    BOOST_CHECK(dat.add(2,"tmp"));
    BOOST_CHECK(dat.contains(2));
    BOOST_CHECK(dat.remove(2));
    BOOST_CHECK(!dat.contains(2));
}

BOOST_AUTO_TEST_CASE(remove_head)
{
    SkipList<int, string,4> dat(INT_MIN,INT_MAX);
    BOOST_CHECK(dat.add(1,"v1"));
    BOOST_CHECK(dat.add(2,"v2"));
    BOOST_CHECK(dat.add(3,"v3"));

    BOOST_CHECK(dat.contains(1));
    BOOST_CHECK(dat.contains(2));
    BOOST_CHECK(dat.contains(3));
    BOOST_CHECK(dat.remove(1));
    BOOST_CHECK(!dat.contains(1));
    BOOST_CHECK(dat.contains(2));
    BOOST_CHECK(dat.contains(3));

}

BOOST_AUTO_TEST_CASE(remove_middle)
{
    SkipList<int, string,4> dat(INT_MIN,INT_MAX);
    BOOST_CHECK(dat.add(1,"v1"));
    BOOST_CHECK(dat.add(2,"v2"));
    BOOST_CHECK(dat.add(3,"v3"));
    BOOST_CHECK(dat.contains(1));
    BOOST_CHECK(dat.contains(2));
    BOOST_CHECK(dat.contains(3));
    BOOST_CHECK(dat.remove(2));
    BOOST_CHECK(dat.contains(1));
    BOOST_CHECK(!dat.contains(2));
    BOOST_CHECK(dat.contains(3));
}

BOOST_AUTO_TEST_CASE(remove_tail)
{
    SkipList<int, string,4> dat(INT_MIN,INT_MAX);
    BOOST_CHECK(dat.add(1,"v1"));
    BOOST_CHECK(dat.add(2,"v2"));
    BOOST_CHECK(dat.add(3,"v3"));
    BOOST_CHECK(dat.contains(1));
    BOOST_CHECK(dat.contains(2));
    BOOST_CHECK(dat.contains(3));
    BOOST_CHECK(dat.remove(3));
    BOOST_CHECK(dat.contains(1));
    BOOST_CHECK(dat.contains(2));
    BOOST_CHECK(!dat.contains(3));
}

BOOST_AUTO_TEST_CASE(empty_test)
{
    SkipList<int, int,4> dat(INT_MIN,INT_MAX);
    BOOST_CHECK(dat.is_empty());
    BOOST_CHECK(dat.add(2,3));
    BOOST_CHECK(!dat.is_empty());
}

BOOST_AUTO_TEST_CASE(get_failue)
{
    SkipList<int, int,4> dat(INT_MIN,INT_MAX);
    for(int i=0; i<10; i++)
    {
        BOOST_CHECK(dat.get(i) == dat.end());
    }
}

BOOST_AUTO_TEST_CASE(get_succeed)
{
    SkipList<int, int,4> dat(INT_MIN,INT_MAX);
    for(int i=0; i<10; i++)
    {
        dat.add(i,i*2);
    }
    for(int i=0; i<10; i++)
    {
        BOOST_CHECK(dat.get(i)->second == i*2);
    }
}


BOOST_AUTO_TEST_CASE(random_add_500)
{
    SkipList<int, int, 6> dat(INT_MIN,INT_MAX);
    for(int i=0; i<500; i++)
    {
        BOOST_CHECK(dat.add(i,3));
    }
    for(int i=0; i<500; i++)
    {
        BOOST_CHECK(dat.contains(i));
    }
    for(int i=0; i<500; i++)
    {
        BOOST_CHECK(dat.remove(i));
    }
    for(int i=0; i<500; i++)
    {
        BOOST_CHECK(!dat.contains(i));
    }
}

BOOST_AUTO_TEST_CASE(iterator_get)
{
    typedef SkipList<int,int,4> SkipList;
    SkipList dat(INT_MIN,INT_MAX);
    dat.add(12,1);
    SkipList::iterator it = dat.get(21);
    BOOST_CHECK(it == dat.end());
    SkipList::iterator jt = dat.get(12);
    BOOST_CHECK(jt->first == 12);
    BOOST_CHECK(jt->second == 1);
}

BOOST_AUTO_TEST_CASE(iterator_inc)
{
    typedef SkipList<int,int,4> SkipList;
    SkipList dat(INT_MIN,INT_MAX);
    dat.add(12,1);
    dat.add(13,1);
    SkipList::iterator it = dat.lower_bound(12);
    BOOST_CHECK(it->first == 12);
    ++it;
    BOOST_CHECK(it->first == 13);
    ++it;
    BOOST_CHECK(it == dat.end());
}

const int nodes = 20000;
typedef SkipList<int,int, 24> skip;

skip skiplist(INT_MIN, INT_MAX, 0);

template<typename rand>
void inserter(int i, rand& rnd, int seed){
    while(seed-->0)rnd();
    while(i>0){
        skiplist.add(rnd(),1);
        --i;
    }
};

template<typename rand>
void deleter(int i, rand& rnd, int seed){
    while(seed-->0)rnd();
    while(i>0){
        int random = rnd();
        skiplist.remove(random);
        //	assert(!skiplist.contains(random));
        --i;
    }
};

BOOST_AUTO_TEST_CASE(Bench)
{
    using namespace boost; 
	
    typedef variate_generator<mt19937&, uniform_int<> > random_type;
    mt19937 gen1(1),gen2(2),gen3(3),gen4(4);
    mt19937 gen5(1),gen6(2),gen7(3),gen8(4);
    uniform_int<> dst(0, 100000000);
	
    boost::timer t;
    boost::thread ins1(boost::bind(&inserter<random_type>, nodes/4, random_type(gen1, dst), 1));
    boost::thread ins2(boost::bind(&inserter<random_type>, nodes/4, random_type(gen2, dst), 2));
    boost::thread ins3(boost::bind(&inserter<random_type>, nodes/4, random_type(gen3, dst), 3));
    boost::thread ins4(boost::bind(&inserter<random_type>, nodes/4, random_type(gen4, dst), 4));
//	usleep(1000000);
    boost::thread del1(boost::bind(&deleter<random_type>, nodes/4, random_type(gen5, dst), 5));
    boost::thread del2(boost::bind(&deleter<random_type>, nodes/4, random_type(gen6, dst), 6));
    boost::thread del3(boost::bind(&deleter<random_type>, nodes/4, random_type(gen7, dst), 7));
    boost::thread del4(boost::bind(&deleter<random_type>, nodes/4, random_type(gen8, dst), 8));

//	skiplist.dump();
    ins1.join();ins2.join();ins3.join();ins4.join();
    del1.join();del2.join();del3.join();del4.join();
	
    int time = t.elapsed();
    if(time == 0) time++;
    //date_time::subsecond_duration<time_duration, long long int>
	
    std::cout << nodes/time << " ops" << std::endl;
}


BOOST_AUTO_TEST_SUITE_END()

