#include <util/DynamicBloomFilter.h>
#include <util/Int2String.h>

#include <boost/test/unit_test.hpp>
#include <boost/timer.hpp>
#include <boost/thread.hpp>

#include <string>
#include <iostream>

using namespace izenelib::util;
using namespace std;


BOOST_AUTO_TEST_CASE(BloomFilter_test)
{
    BloomFilter<std::string> bloomfiler(10000,0.01);
    for(unsigned i =0; i < 100; ++i)
    {
        std::string key(Int2String(i).x);
        bloomfiler.Insert(key); 
    }
    for(unsigned i =0; i < 100; ++i)
    {
        std::string key(Int2String(i).x);
        BOOST_CHECK(bloomfiler.Get(key)); 
    }
    for(unsigned i =101; i < 200; ++i)
    {
        std::string key(Int2String(i).x);
        BOOST_CHECK(!bloomfiler.Get(key)); 
    }
}
typedef BloomFilter<std::string> BloomFilterType;
MAKE_FEBIRD_SERIALIZATION(BloomFilterType )

BOOST_AUTO_TEST_CASE(BloomFilter_storage_test)
{
    BloomFilterType bloomfiler(10000,0.01);
    for(unsigned i =0; i < 100; ++i)
    {
        std::string key(Int2String(i).x);
        bloomfiler.Insert(key); 
    }

    izene_serialization_febird<BloomFilterType > isb(bloomfiler);
    char* ptr;
    size_t sz;
    isb.write_image(ptr, sz);
    BloomFilterType inFilter;

    izene_deserialization_febird<BloomFilterType > idb(ptr, sz);
    idb.read_image(inFilter);

    for(unsigned i =0; i < 100; ++i)
    {
        std::string key(Int2String(i).x);
        BOOST_CHECK(inFilter.Get(key)); 
    }
    for(unsigned i =101; i < 200; ++i)
    {
        std::string key(Int2String(i).x);
        BOOST_CHECK(!inFilter.Get(key)); 
    }
}

BOOST_AUTO_TEST_CASE(DynamicBloomFilter_test)
{
    DynamicBloomFilter<std::string> bloomfiler(10000,0.01,10);
    for(unsigned i =0; i < 100; ++i)
    {
        std::string key(Int2String(i).x);
        bloomfiler.Insert(key); 
    }
    for(unsigned i =0; i < 100; ++i)
    {
        std::string key(Int2String(i).x);
        BOOST_CHECK(bloomfiler.Get(key)); 
    }
    for(unsigned i =101; i < 200; ++i)
    {
        std::string key(Int2String(i).x);
        BOOST_CHECK(!bloomfiler.Get(key)); 
    }
}

typedef DynamicBloomFilter<std::string> DynamicBloomFilterType;
MAKE_FEBIRD_SERIALIZATION(DynamicBloomFilterType )

BOOST_AUTO_TEST_CASE(DynamicBloomFilter_storage_test)
{
    DynamicBloomFilterType bloomfiler(10000,0.01,10);
    for(unsigned i =0; i < 100; ++i)
    {
        std::string key(Int2String(i).x);
        bloomfiler.Insert(key); 
    }

    izene_serialization_febird<DynamicBloomFilterType > isb(bloomfiler);
    char* ptr;
    size_t sz;
    isb.write_image(ptr, sz);
    DynamicBloomFilterType inFilter;

    izene_deserialization_febird<DynamicBloomFilterType > idb(ptr, sz);
    idb.read_image(inFilter);

    for(unsigned i =0; i < 100; ++i)
    {
        std::string key(Int2String(i).x);
        BOOST_CHECK(inFilter.Get(key)); 
    }
    for(unsigned i =101; i < 200; ++i)
    {
        std::string key(Int2String(i).x);
        BOOST_CHECK(!inFilter.Get(key)); 
    }
}
