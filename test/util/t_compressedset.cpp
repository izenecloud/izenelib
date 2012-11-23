#include <time.h>

#include  <util/compression/int/compressedset/CompressedSet.h>
#include  <util/compression/int/compressedset/LazyAndSet.h>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace izenelib::util::compression;

BOOST_AUTO_TEST_CASE(compressedset_small)
{
    CompressedSet myset1;
    unsigned count = 10;
    for (unsigned int i = 1; i<=count; ++i)
    {
        myset1.addDoc(i);
    }
    std::cout << "compressed!" << endl;

    myset1.flush();
    myset1.compact();
    std::cout << "size "<<myset1.size()<< endl;

    CompressedSet::Iterator it(&myset1);
    unsigned i = 1;
    while(it.nextDoc() && it.docID() != NO_MORE_DOCS)
    {
        std::cout<<"doc "<<it.docID()<<std::endl;
        BOOST_CHECK(i == it.docID());
        ++i;
    }
}

bool testvec(set<unsigned int>& data)
{
    stringstream ss;
    {
        CompressedSet myset2;
        for (set<unsigned int>::iterator dit = data.begin(); dit != data.end(); ++dit)
        {
            myset2.addDoc(*dit);
        }
        // cout << "added " << data.size() << endl;
        myset2.flush();
        myset2.compact();
        myset2.write(ss);
    }

    CompressedSet myset1;
    myset1.read(ss);

    assert(data.size() == myset1.size());
    CompressedSet::Iterator it2(&myset1);
    for (set<unsigned int>::iterator dit = data.begin(); dit != data.end(); ++dit)	
    {
        assert(it2.nextDoc()!=NO_MORE_DOCS );
        assert(it2.docID() == *dit);
    }
    assert(it2.nextDoc() == NO_MORE_DOCS);

    return true;
}

BOOST_AUTO_TEST_CASE(compressedset_big)
{
    for (uint32_t b = 0; b <= 28; ++b)
    {
        cout << "testing1... b = " << b << endl;
        for (size_t length = 128; length < (1U << 12); length += 128)
        {
            //cout << "   length = " << length << endl;
            set<unsigned int> data;
            for (size_t i = 0; i < length; ++i)
            {
                unsigned int x = (i + (24 - i) * (12 - i)) % (1U << b);
                data.insert(x);
            }
            if (!testvec(data))
            {
                return;
            }
        }
        cout << "testing2... b = " << b << endl;
        for (size_t length = 1; length < (1U << 9); ++length)
        {
            //  cout << "   length = " << length << endl;
            set<unsigned int> data;
            for (size_t i = 0; i < length; ++i)
            {
                data.insert((33231 - i + i * i) % (1U << b));
            }
            if (!testvec(data))
            {
                return;
            }
        }
    }
    cout << "All test passed succesfully!!" << endl;
}

