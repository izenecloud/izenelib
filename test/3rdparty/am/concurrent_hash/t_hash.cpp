#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>

#include <concurrent_hash/hashmap.h>
#include <string>
#include <list>

using boost::thread;
using boost::bind;
using boost::array;
using namespace concurrent;

BOOST_AUTO_TEST_SUITE(concurrent_hash_test)

BOOST_AUTO_TEST_CASE(hash_operation)
{
    {
        hashmap<int, char> hmp;
        hmp.insert(std::make_pair<int,char>(2,4));
        BOOST_CHECK(hmp.contains(2));
    }
    {
        hashmap<int, char> hmp;
        hmp.insert(std::make_pair<int,char>(2,4));
        hmp.insert(std::make_pair<int,char>(3,4));
        BOOST_CHECK(hmp.contains(2));
        BOOST_CHECK(hmp.contains(2));
    }
    {
        hashmap<int, int> hmp;
        for (int i=0; i < 256; ++i)
        {
            hmp.insert(std::make_pair<int,int>(i+0,i*i));
        }
        for (int i=0; i < 256; ++i)
        {
            BOOST_CHECK(hmp.contains(i));
        }
    }
    {
        hashmap<int, char> hmp;
        hmp.insert(std::make_pair(1,2));
        BOOST_CHECK_EQUAL(hmp.get(1),2);
    }
    {
        hashmap<int, int> hmp;
        for (int i=0; i < 256; ++i)
        {
            hmp.insert(std::make_pair(i,i));
        }
        for (int i=0; i < 256; ++i)
        {
            BOOST_CHECK_EQUAL(hmp.get(i), i);
        }
    }
    {
        hashmap<int, char> hmp;
        hmp.insert(std::make_pair(1,2));
        BOOST_CHECK(hmp.contains(1));
        hmp.remove(1);
        BOOST_CHECK_EQUAL(hmp.contains(1),false);
    }
    {
        hashmap<int, int> hmp;
        for (int i=0; i < 1024; ++i)
        {
            hmp.insert(std::make_pair(i+0,i*i));
        }
        for (int i=0; i < 1024; i+=2)
        {
            hmp.remove(i);
        }
        for (int i=0; i < 1024; ++i)
        {
            if (i&1)
            {
                BOOST_CHECK(hmp.contains(i));
            }
            else
            {
                BOOST_CHECK_EQUAL(hmp.contains(i),false);
            }
        }
    }
}

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
void remove_worker(hashmap<key,value>* target
                   ,boost::barrier* b
                   ,const std::vector<key> keys)
{
    b->wait();
    for (size_t i=0 ; i < keys.size(); ++i)
    {
        target->remove(keys[i]);
    }
}
template<typename key, typename value>
void get_worker(hashmap<key,value>* target
                ,boost::barrier* b
                ,const std::vector<key> keys)
{
    b->wait();
    for (size_t i=0 ; i < keys.size(); ++i)
    {
        try
        {
            target->get(keys[i]);
        }
        catch (not_found e) {};
    }
}

BOOST_AUTO_TEST_CASE(concurrent_hash_operation)
{
    {
        const int testnum = 512;
        std::vector<int> keys_1, keys_2, values_1, values_2;
        for (int i=0;i<testnum;i++)
        {
            keys_1.push_back(i);
            keys_2.push_back(i+testnum);
            values_1.push_back(i*i);
            values_2.push_back((i+testnum) * (i+testnum));
        }
        hashmap<int, int> hmp;
        boost::barrier bar(2);
        boost::thread a(boost::bind(insert_worker<int,int>
                                    , &hmp, &bar, keys_1, values_1))
        , b(boost::bind(insert_worker<int,int>
                        , &hmp, &bar, keys_2, values_2));
        a.join();
        b.join();
    }

    {
        std::vector<int> keys_1, keys_2, values_1, values_2;
        const int test = 4096;
        for (int i=0;i<test;i++)
        {
            keys_1.push_back(i);
            keys_2.push_back(i+test);
            values_1.push_back(i*i);
            values_2.push_back((i+test) * (i+test));
        }
        hashmap<int, int> hmp;
        boost::barrier bar(2);
        thread a(boost::bind(insert_worker<int,int>
                             , &hmp, &bar, keys_1, values_1))
        , b(boost::bind(insert_worker<int,int>
                        , &hmp, &bar,  keys_2, values_2));
        a.join();
        b.join();
        for (int i=0;i<test;i++)
        {
            BOOST_CHECK(hmp.contains(i));
            BOOST_CHECK(hmp.contains(i + test));
        }
    }

    {
        std::vector<int> keys_1, keys_2, values_1, values_2;
        const int testsize = 4096;
        hashmap<int, int> hmp;
        for (int i=0;i<testsize;i++)
        {
            hmp.insert(std::make_pair(i,i));
            hmp.insert(std::make_pair(i+testsize, (i+testsize)*(i+testsize)));
            keys_1.push_back(i);
            keys_2.push_back(i+testsize);
        }
        boost::barrier bar(2);
        thread a(boost::bind(remove_worker<int,int>
                             , &hmp, &bar, keys_1))
        , b(boost::bind(remove_worker<int,int>
                        , &hmp, &bar,  keys_2));
        a.join();
        b.join();
        for (int i=0;i<testsize;i++)
        {
            BOOST_CHECK_EQUAL(hmp.contains(i),false);
            BOOST_CHECK_EQUAL(hmp.contains(i + testsize),false);
        }
    }

    {
        std::vector<int> keys_1, keys_2, values_1, values_2;
        const int testsize = 4096;
        hashmap<int, int> hmp;
        for (int i=0;i<testsize;i++)
        {
            keys_1.push_back(i*2); // to remove
            hmp.insert(std::make_pair(i*2, i));
            keys_2.push_back(i*2+1); // to insert
            values_2.push_back((i+testsize) * (i+testsize)); // to insert
        }

        boost::barrier bar(2);
        thread
        a(bind(remove_worker<int,int>, &hmp, &bar, keys_1)),
        b(bind(insert_worker<int,int>, &hmp, &bar,  keys_2, values_2));
        a.join();
        b.join();
        for (int i=0;i<testsize;i++)
        {
            BOOST_CHECK_EQUAL(hmp.contains(i*2),false);
            BOOST_CHECK(hmp.contains(i*2 + 1));
        }
    }

    {
        boost::array<std::vector<int>, 4> keys, values;
        hashmap<int, int> hmp;
        const int testsize = 4096;
        for (int i = 0; i<4; ++i)
        {
            keys[i].resize(testsize);
            values[i].resize(testsize);
        }
        for (int i=0;i<testsize;i++)
        {
            keys[0][i] = i*4; // to remove
            keys[1][i] = i*4 + 1; // to insert
            keys[2][i] = i*4 + 2; // to remove
            keys[3][i] = i*4 + 3; // to insert
            values[1][i] = i;
            values[3][i] = i;
        }
        boost::barrier bar(4);
        thread
        a(bind(remove_worker<int,int>, &hmp, &bar, keys[0])),
        b(bind(insert_worker<int,int>, &hmp, &bar, keys[1], values[1])),
        c(bind(remove_worker<int,int>, &hmp, &bar, keys[2])),
        d(bind(insert_worker<int,int>, &hmp, &bar, keys[3], values[3]));
        a.join();
        b.join();
        c.join();
        d.join();
        for (int i=0;i<testsize;i++)
        {
            BOOST_CHECK_EQUAL(hmp.contains(i*4),false);
            BOOST_CHECK(hmp.contains(i*4 + 1));
            BOOST_CHECK_EQUAL(hmp.contains(i*4 + 2),false);
            BOOST_CHECK(hmp.contains(i*4 + 3));
        }
    }

    {
        boost::array<std::vector<int>, 4> keys, values;
        hashmap<int, int> hmp;
        const int testsize = 4096;
        for (int i = 0; i<4; ++i)
        {
            keys[i].resize(testsize);
            values[i].resize(testsize);
        }
        for (int i=0;i<testsize;i++)
        {
            keys[0][i] = i*4;
            hmp.insert(std::make_pair(i*4, i*4 + 2)); // to get
            keys[1][i] = i*4 + 1; // to insert
            keys[2][i] = i*4 + 2; // to get
            hmp.insert(std::make_pair(i*4 + 2, i*4 + 3));
            keys[3][i] = i*4 + 3; // to insert
            values[1][i] = i;
            values[3][i] = i;
        }
        boost::barrier bar(4);
        thread a[] =
        {
            thread(bind(get_worker<int,int>, &hmp, &bar, keys[0])),
            thread(bind(insert_worker<int,int>, &hmp, &bar, keys[1], values[1])),
            thread(bind(get_worker<int,int>, &hmp, &bar, keys[2])),
            thread(bind(insert_worker<int,int>, &hmp, &bar, keys[3], values[3]))
        };
        for (int i=0;i<4;++i)
        {
            a[i].join();
        }
        for (int i=0;i<testsize;i++)
        {
            BOOST_CHECK(hmp.contains(i*4));
            BOOST_CHECK(hmp.contains(i*4 + 1));
            BOOST_CHECK(hmp.contains(i*4 + 2));
            BOOST_CHECK(hmp.contains(i*4 + 3));
        }
    }

    {
        boost::array<std::vector<int>, 4> keys;
        hashmap<int, int> hmp;
        const int testsize = 4096;
        for (int i = 0; i<4; ++i)
        {
            keys[i].resize(testsize);
        }
        for (int i=0;i<testsize;i++)
        {
            keys[0][i] = i*4;
            hmp.insert(std::make_pair(i*4, i*4 + 1)); // to get
            keys[1][i] = i*4 + 1;
            hmp.insert(std::make_pair(i*4 + 1, i*4 + 2)); // to remove
            keys[2][i] = i*4 + 2;
            hmp.insert(std::make_pair(i*4 + 2, i*4 + 3)); // to get
            keys[3][i] = i*4 + 3;
            hmp.insert(std::make_pair(i*4 + 3, i*4 + 4)); // to remove
        }
        boost::barrier bar(4);
        thread a[] =
        {
            thread(bind(get_worker<int,int>, &hmp, &bar, keys[0])),
            thread(bind(remove_worker<int,int>, &hmp, &bar, keys[1])),
            thread(bind(get_worker<int,int>, &hmp, &bar, keys[2])),
            thread(bind(remove_worker<int,int>, &hmp, &bar, keys[3]))
        };
        for (int i=0;i<4;++i)
        {
            a[i].join();
        }
        for (int i=0;i<testsize;i++)
        {
            BOOST_CHECK(hmp.contains(i*4));
            BOOST_CHECK_EQUAL(hmp.contains(i*4 + 1),false);
            BOOST_CHECK(hmp.contains(i*4 + 2));
            BOOST_CHECK_EQUAL(hmp.contains(i*4 + 3),false);
        }
    }

    {
        boost::array<std::vector<int>, 6> keys, values;
        hashmap<int, int> hmp;
        const int testsize = 4096;
        for (int i = 0; i<6; ++i)
        {
            keys[i].resize(testsize);
            values[i].resize(testsize);
        }
        for (int i=0;i<testsize;i++)
        {
            keys[0][i] = i*6; // to insert
            values[0][i] = i*6 + 1;
            keys[1][i] = i*6 + 1;
            hmp.insert(std::make_pair(i*6 + 1, i*6 + 2)); // to get
            keys[2][i] = i*6 + 2;
            hmp.insert(std::make_pair(i*6 + 2, i*6 + 3)); // to remove
            keys[3][i] = i*6 + 3; // to insert
            values[3][i] = i*6 + 4;
            keys[4][i] = i*6 + 4;
            hmp.insert(std::make_pair(i*6 + 4, i*6 + 5)); // to get
            keys[5][i] = i*6 + 5;
            hmp.insert(std::make_pair(i*6 + 5, i*6 + 6)); // to remove
        }
        boost::barrier bar(6);
        thread a[] =
        {
            thread(bind(insert_worker<int,int>, &hmp, &bar, keys[0], values[0])),
            thread(bind(get_worker<int,int>, &hmp, &bar, keys[1])),
            thread(bind(remove_worker<int,int>, &hmp, &bar, keys[2])),
            thread(bind(insert_worker<int,int>, &hmp, &bar, keys[3], values[3])),
            thread(bind(get_worker<int,int>, &hmp, &bar, keys[4])),
            thread(bind(remove_worker<int,int>, &hmp, &bar, keys[5]))
        };
        for (int i=0;i<6;++i)
        {
            a[i].join();
        }
        for (int i=0;i<testsize;i++)
        {
            BOOST_CHECK(hmp.contains(i*6));
            BOOST_CHECK(hmp.contains(i*6 + 1));
            BOOST_CHECK_EQUAL(hmp.contains(i*6 + 2),false);
            BOOST_CHECK(hmp.contains(i*6 + 3));
            BOOST_CHECK(hmp.contains(i*6 + 4));
            BOOST_CHECK_EQUAL(hmp.contains(i*6 + 5),false);
        }
    }

    {
        boost::array<std::vector<int>, 6> keys, values;
        hashmap<int, int> hmp;
        const int testsize = 4096;
        for (int i = 0; i<6; ++i)
        {
            keys[i].resize(testsize);
            values[i].resize(testsize);
        }
        for (int i=0;i<testsize;i++)
        {
            for (int j=0;j<6;++j)
            {
                keys[j][i] = i*6 + j;
                values[0][i] = i*i*6 + 1;
            }
            hmp.insert(std::make_pair(i*6 + 1, i*6 + 2)); // to get
            hmp.insert(std::make_pair(i*6 + 2, i*6 + 3)); // to remove
            hmp.insert(std::make_pair(i*6 + 4, i*6 + 5)); // to get
            hmp.insert(std::make_pair(i*6 + 5, i*6 + 6)); // to remove
        }
        boost::barrier bar(6);
        thread a[] =
        {
            thread(bind(insert_worker<int,int>, &hmp, &bar, keys[0], values[0])),
            thread(bind(get_worker<int,int>, &hmp, &bar, keys[1])),
            thread(bind(remove_worker<int,int>, &hmp, &bar, keys[2])),
            thread(bind(insert_worker<int,int>, &hmp, &bar, keys[3], values[3])),
            thread(bind(get_worker<int,int>, &hmp, &bar, keys[4])),
            thread(bind(remove_worker<int,int>, &hmp, &bar, keys[5]))
        };
        for (int i=0;i<6;++i)
        {
            a[i].join();
        }
        for (int i=0;i<testsize;i++)
        {
            BOOST_CHECK(hmp.contains(i*6));
            BOOST_CHECK(hmp.contains(i*6 + 1));
            BOOST_CHECK_EQUAL(hmp.contains(i*6 + 2),false);
            BOOST_CHECK(hmp.contains(i*6 + 3));
            BOOST_CHECK(hmp.contains(i*6 + 4));
            BOOST_CHECK_EQUAL(hmp.contains(i*6 + 5),false);
        }
    }

}
BOOST_AUTO_TEST_SUITE_END() // concurrent_hash_test

