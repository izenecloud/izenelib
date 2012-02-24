#include<am/concurrent/slfvector.h>
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include<iostream>
#include<stdexcept>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include<fstream>

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
    std::cout<<time(NULL)<<std::endl;
    for(int i = 0; i < 100000000; ++i){
        vc.push_back(i);
    }
    std::cout<<time(NULL)<<std::endl;
    for(int j = 0; j < 10; ++j)
    for(int i = 0; i < 100000000; ++i){
        vc.at(i);
        vc[i];
    }
    std::cout<<time(NULL)<<std::endl;
}

BOOST_AUTO_TEST_CASE(resize){
    slfvector<int> vc(0);
    vc.clear();
    for(int i = 0; i < 100; ++i){
        vc.push_back(i);
    }
    vc.resize(10);
    BOOST_CHECK(vc.size() == 10);
    vc.resize(200);
    BOOST_CHECK(vc.size() == 200);
    vc.resize(0);
    BOOST_CHECK(vc.size() == 0);
    vc.clear();
    vc.resize(1);
    BOOST_CHECK(vc.size() == 1);
}

BOOST_AUTO_TEST_CASE(copy_constructor){
    slfvector<int> vc;
    for(int i = 0; i < 20; ++i){
        vc.push_back(i);
    }
    {
        slfvector<int>vc2(vc);
        for(uint32_t idx = 0; idx < 20; ++idx)
            BOOST_CHECK(vc.at(idx) == vc2.at(idx));
        vc = vc2;
    }
    BOOST_CHECK(vc.size() == 20);
    vc.clear();
}

BOOST_AUTO_TEST_CASE(swap){
    slfvector<int> vec1, vec2;
    for(int i = 0; i < 10; ++i){
        vec1.push_back(i);
        vec2.push_back(100 + i);
    }
    vec1.swap(vec2);
    for(int i = 0; i < 10; ++i){
        std::cout<<"Swap: "<<vec1[i]<<endl;
    }
    for(int i = 0; i < 10; ++i){
        std::cout<<"Swap 2: "<<vec2[i]<<endl;
    }
}

BOOST_AUTO_TEST_CASE(serialize){
    std::ofstream ofs("filename");
    slfvector<int> vec;
    for(int i = 0; i < 10; ++i)
        vec.push_back(i);
    {
        boost::archive::text_oarchive oa(ofs);
        oa << vec;
    }
    std::cout<<"Serialized!"<<std::endl;
    slfvector<int> vec2;
    {
        std::ifstream ifs("filename");
        boost::archive::text_iarchive ia(ifs);
        ia >> vec2;
    }
    std::cout<<"Deserialized!"<<std::endl;
    std::cout<<"Test serialize, size: "<<vec2.size()<<std::endl;
    for(int i = 1; i < 10; ++i)
        std::cout<<"Test serialize: "<<vec2.at(i)<<endl;
}

slfvector<int> vcbench;
void push(){
    for(int i = 0; i < 100000; ++i){
        vcbench.push_back(i);
    }
}

void read(){
    for(int i = 0; i < 10000; ++i){
        try{
            vcbench.at((i * 97) % (vcbench.size() + 1) - 1);
        }catch(const std::out_of_range& e){
        }
    }
}

BOOST_AUTO_TEST_CASE(Bench){
slfvector<int> vcbench;
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
