#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/timer.hpp>

#include <concurrent_hash/hashmap.h>
#include <vector>
#include <assert.h>
#include <stdio.h>
#include <iostream>

using namespace concurrent;

template<typename key, typename value>
void insert_worker(hashmap<key,value>* target
                   ,boost::barrier* b
                   ,const std::vector<key> keys
                   , const std::vector<value> values)
{
    assert(keys.size() == values.size());
    b->wait();
    for (size_t i=0 ; i < keys.size(); ++i)
    {
        target->insert(std::make_pair(keys[i],values[i]));

    }
}


template<typename key, typename value>
void contains_worker(hashmap<key,value>* target
                     ,boost::barrier* b
                     ,const std::vector<key> keys)
{
    b->wait();
    for (size_t i=0 ; i < keys.size(); ++i)
    {
        target->contains(keys[i]);
    }
}

BOOST_AUTO_TEST_SUITE(concurrent_hash_test)

BOOST_AUTO_TEST_CASE(insert_bench)
{
    const int testsize = 1000;
    const int threads = 3;
    const int locks = 3;
    const int max_chain = 3;

    //hashmap<int, int> hmp(locks, 11,max_chain);
    hashmap<int, int> hmp(locks);

    std::vector<std::vector<int> > keys,values;
    for (int i = 0; i<threads; ++i)
    {
        keys.resize(threads);
        values.resize(threads);
    }
    for (int i=0;i<testsize;i++)
    {
        const int target = i % threads;
        keys[target].push_back(i);
        values[target].push_back(i*i);
    }
    boost::timer time;
    boost::barrier bar(threads);
    boost::thread_group tg;
    for (int i=0;i<threads;++i)
    {
        tg.create_thread(bind(insert_worker<int,int>, &hmp, &bar, keys[i], values[i]));
    }
    tg.join_all();
    double elapsed = time.elapsed();
    printf("insert:%d items by %d threads %d locks %f q/s\n",
           testsize, threads, locks, (double)testsize/elapsed);
    //hmp.dump();
}


BOOST_AUTO_TEST_CASE(search_bench)
{
    const int testsize = 1000;
    const int threads = 3;
    const int locks = 3;
    const int max_chain = 3;
    hashmap<int, int> hmp(locks, 11, max_chain);

    std::vector<std::vector<int> > keys;
    for (int i = 0; i<threads; ++i)
    {
        keys.resize(threads);
    }
    for (int i=0;i<testsize;i++)
    {
        const int target = i % threads;
        keys[target].push_back(i);
        hmp.insert(std::make_pair(i, i * i));
    }
    boost::timer time;
    boost::barrier bar(threads);
    boost::thread_group tg;
    for (int i=0;i<threads;++i)
    {
        tg.create_thread(bind(contains_worker<int,int>, &hmp, &bar, keys[i]));
    }
    tg.join_all();
    double elapsed = time.elapsed();
    printf("contains:%d items by %d threads %d locks %f q/s\n",
           testsize, threads, locks, (double)testsize/elapsed);
    //hmp.dump();
}

BOOST_AUTO_TEST_SUITE_END() // concurrent_hash_test

