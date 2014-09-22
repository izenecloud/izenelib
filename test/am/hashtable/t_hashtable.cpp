/*  #include <am/hashtable/cuckoo_hash_table.hpp>
#include <am/hashtable/coalease_hash_table.hpp>

#include <boost/test/unit_test.hpp>
#include <stdlib.h>
#include <sys/time.h>

#include <string>
#include <time.h>
#include <math.h>

#include <sys/time.h>
#include <fstream>
#include <iostream>
#include <vector>
#include<stdio.h>

using namespace izenelib::am;
using namespace std;

uint32_t error_count = 0;

BOOST_AUTO_TEST_SUITE(hash_test)

static int data_size = 1000000;
static unsigned *int_data;

void init_data()
{
    int i;
    std::cout<<"generating data... "<<std::endl;
    srand48(11);
    int_data = (unsigned*)calloc(data_size, sizeof(unsigned));
    for (i = 0; i < data_size; ++i) {
        int_data[i] = (unsigned)(data_size * drand48() / 4) * 271828183u;
    }
    std::cout<<"done!\n";
}

void destroy_data()
{
    free(int_data);
}

BOOST_AUTO_TEST_CASE(cuckoo_test)
{
    CuckooHashTable<int,double> table(6,1,1);

    int i;

    init_data();
    int size = 300;
    for (i = 1; i < size; ++i) {
        table.insert(i,(double)int_data[i]);
    }
    cout<<"insert finished"<<endl;
    for (i = 1; i < size; ++i) {
	double v = table.find(i);
	BOOST_CHECK(v == (double)int_data[i]);
    }

    cout<<"occupy percentage "<<table.occupy()<<endl;
    destroy_data();
}

BOOST_AUTO_TEST_CASE(coalease_test)
{
    CoaleaseHashTable<int,double> table;

    int i;
    init_data();
    int size = 700;
    for (i = 1; i < size; ++i) {
        table.insert(i,(double)int_data[i]);
    }
    cout<<"insert finished"<<endl;
    for (i = 1; i < size; ++i) {
	double v = table[i];
	BOOST_CHECK(v == (double)int_data[i]);
    }

    cout<<"occupy percentage "<<table.occupy()<<endl;
    destroy_data();
}

BOOST_AUTO_TEST_SUITE_END()
*/ 
