#include<am/concurrent/slfvector.h>
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include<iostream>

using namespace std;
using namespace izenelib::am::concurrent;


BOOST_AUTO_TEST_SUITE( t_concurrent_read_vector_suite )

BOOST_AUTO_TEST_CASE(push_back){
    slfvector<int> vc;
    for(int i = 0; i < 100; ++i){
        vc.push_back(i);
    }
    BOOST_CHECK(vc.size() == 100);
}

BOOST_AUTO_TEST_CASE(at){
    slfvector<int> vc;
    for(int i = 0; i < 100; ++i){
        vc.push_back(i);
    }
    for(int i = 0; i < 100; ++i){
        BOOST_CHECK(vc.at(i) == i);
        BOOST_CHECK(vc[i] == i);
    }
}

BOOST_AUTO_TEST_CASE(reserve){
    slfvector<int> vc;
    for(int i = 0; i < 100; ++i){
        vc.push_back(i);
    }
    vc.reserve(10);
    BOOST_CHECK(vc.size() == 10);
}

slfvector<int> vcbench;
void push(){
    for(int i = 0; i < 100000; ++i){
        vcbench.push_back(i);
    }
}

void read(){
    for(int i = 0; i < 10000; ++i){
        vcbench.at((i * 97) % vcbench.size());
    }
}

BOOST_AUTO_TEST_CASE(Bench){
    boost::thread push_t1(boost::bind(&push));
    boost::thread read_t2(boost::bind(&read));
    boost::thread read_t3(boost::bind(&read));
    boost::thread read_t4(boost::bind(&read));
    boost::thread read_t5(boost::bind(&read));
    boost::thread read_t6(boost::bind(&read));
    boost::thread read_t7(boost::bind(&read));
    boost::thread read_t8(boost::bind(&read));
    boost::thread read_t9(boost::bind(&read));
    boost::thread read_t10(boost::bind(&read));

    push_t1.join();
    read_t2.join();
    read_t3.join();
    read_t4.join();
    read_t5.join();
    read_t6.join();
    read_t7.join();
    read_t8.join();
    read_t9.join();
    read_t10.join();
}

BOOST_AUTO_TEST_SUITE_END()
