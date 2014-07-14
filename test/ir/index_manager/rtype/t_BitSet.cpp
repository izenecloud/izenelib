// #ifndef IZENELIB_IR_BITSET_TEST_H
// #define IZENELIB_IR_BITSET_TEST_H

#include <ir/index_manager/utility/Bitset.h>
#include <boost/test/unit_test.hpp>
#include <vector>
#include <iostream>
#include <ctime>
#include <util/ClockTimer.h>

using namespace izenelib::ir::indexmanager;

BOOST_AUTO_TEST_SUITE(t_BitSet)

BOOST_AUTO_TEST_CASE(BITSET_MODEL_1)
{
    // Original Data Model 1: key 1 - 100; each got Value: 0.5M to 2M ;
    srand( (unsigned)time(NULL));

    std::vector<std::vector<int> > DataTable_1(100);
    unsigned int total_Number = 0;
    for (unsigned int i = 0; i < 100; ++i)
    {
        if (i % 10 == 0)
            std::cout << i << std::endl;
        
        int number = rand()%1000000;
        number += 400000;
        total_Number += number;
        int lastDocid = 0;
        for (unsigned int j = 0; j < number; ++j)
        {
            int r = rand()%120;
            lastDocid += r;
            DataTable_1[i].push_back(lastDocid);
        }
	//std::cout << "MAX DOCID: " << lastDocid << std::endl;
    }
    std::cout << "total_Number: " << total_Number/10000 << std::endl;
    // Test 1; MAKE ALL THOSE TO A BIG BITSET;  
    Bitset bitset;
    izenelib::util::ClockTimer timer1;
    for (int i = 0; i < 100; ++i)
    {
        for (int j = 0; j < DataTable_1[i].size(); ++j)
        {
            bitset.set(DataTable_1[i][j]);
        }
    }
    std::cout << "Time cost for DOCLIST type" << timer1.elapsed() << " seconds" << std::endl;
    std::vector<Bitset> bitset_list_bucket(10);
    //1 - 10
    //10 - 20
    //20 - 30
    //30 - 40
    //40 - 50
    //50 - 60
    //60 - 70
    //80 - 90
    //100 - 110
    // ....
    for (int i = 0; i < 10; ++i)
    {
        for (int x = 0; x < 10; ++x)
        {
            for (int j = 0; j < DataTable_1[i * 10 + x].size(); ++j)
            {
                bitset_list_bucket[i].set(DataTable_1[i * 10 + x][j]);
            }
        }
    }

    izenelib::util::ClockTimer timer3;
    Bitset bitset_new;
    for (int i = 0; i < 10; ++i)
    {
        bitset_new |= bitset_list_bucket[i];
    }
    std::cout << "Time cost for BITSET bucket type" << timer3.elapsed() << " seconds" << std::endl;
 
    // Test 2: for 8k BitSet
    //izenelib::util::ClockTimer timer2;
    std::vector<Bitset> bitset_list(100);
    for (int i = 0; i < 100; ++i)
    {
        for (int j = 0; j < DataTable_1[i].size(); ++j)
        {
            bitset_list[i].set(DataTable_1[i][j]);
        }
    }
    std::cout << "--------";
    
    izenelib::util::ClockTimer timer2;
    Bitset bitset_new_2;
    for (int i = 0; i < 100; ++i)
    {
        bitset_new_2 |= bitset_list[i];
    }
    std::cout << "Time cost for BITSET type" << timer2.elapsed() << " seconds" << std::endl << std::endl;


    //for 16k BitSet
    Bitset bitset_16k(16*1024);

    //for 32k BitSet
    Bitset bitset_32k(32*1024);

    //for 64k BitSet
    Bitset bitset_64k(64*1024);
}

BOOST_AUTO_TEST_CASE(BITSET_MODEL_2)
{
    //Original Data Model 1: key 1 - 3000; each got Value: 0.05M to 0.5M ;
    srand( (unsigned)time(NULL));
    unsigned int total_Number = 0;
    std::vector<std::vector<int> > DataTable_2(300);
    for (unsigned int i = 0; i < 300; ++i)
    {
        int number = rand()%400000;
        number += 50000;
        int lastDocid = 0;
        for (unsigned int j = 0; j < number; ++j)
        {
            int r = rand()%400;
            lastDocid += r;
            DataTable_2[i].push_back(lastDocid);
        }
	total_Number += number;
        //std::cout << "MAX DOCID: " << lastDocid << std::endl;
    }
    std::cout << "total_Number: " << total_Number/10000 << std::endl;
    

    std::vector<Bitset> bitset_list_bucket(30);
    //1 - 10
    //10 - 20
    //20 - 30
    //30 - 40
    //40 - 50
    //50 - 60
    //60 - 70
    //80 - 90
    //100 - 110
    // ....
    for (int i = 0; i < 30; ++i)
    {
        for (int x = 0; x < 10; ++x)
        {
            for (int j = 0; j < DataTable_2[i * 10 + x].size(); ++j)
            {
                bitset_list_bucket[i].set(DataTable_2[i * 10 + x][j]);
            }
        }
    }

    izenelib::util::ClockTimer timer3;
    Bitset bitset_new;
    for (int i = 0; i < 30; ++i)
    {
        bitset_new |= bitset_list_bucket[i];
    }
    std::cout << "Time cost for BITSET bucket type" << timer3.elapsed() << " seconds" << std::endl;
 
    // test
    Bitset bitset;
    izenelib::util::ClockTimer timer1;
    for (int i = 0; i < 300; ++i)
    {
        for (int j = 0; j < DataTable_2[i].size(); ++j)
        {
            bitset.set(DataTable_2[i][j]);
        }
    }
    std::cout << "Time cost for DOCLIST type" << timer1.elapsed() << " seconds" << std::endl;

    // test 1; MAKE ALL THOSE TO A BIG BITSET;
    // Test 2: for 8k BitSet
    //izenelib::util::ClockTimer timer2;
    std::vector<Bitset> bitset_list(300);
    for (int i = 0; i < 300; ++i)
    {
        for (int j = 0; j < DataTable_2[i].size(); ++j)
        {
            bitset_list[i].set(DataTable_2[i][j]);
        }
    }

    izenelib::util::ClockTimer timer2;
    Bitset bitset_new_2;
    for (int i = 0; i < 300; ++i)
    {
        bitset_new_2 |= bitset_list[i];
    }
    std::cout << "Time cost for BITSET type" << timer2.elapsed() << " seconds" << std::endl;
    //for 64k BitSet
    Bitset bitset_64k(64*1024);

    //for 128
    Bitset bitset_128k(128*1024);

    //for 256 BitSet
    Bitset bitset_256k(256*1024);
}

BOOST_AUTO_TEST_SUITE_END()

// #endif
