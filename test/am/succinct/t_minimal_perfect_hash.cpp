#include <am/succinct/minimal_perfect_hash.hpp>

#include <cstdlib>
#include <sstream>
#include <vector>
#include <set>
#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

template<typename Hash, typename Key>
void CheckHashValues(const Hash &hash, const std::vector<Key> &keys)
{
    std::vector<uint32_t> h(keys.size());
    for (size_t i = 0; i < keys.size(); ++i)
    {
        h[i] = hash.GetHash(keys[i]);
        BOOST_CHECK(0 <= h[i]);
        BOOST_CHECK((unsigned)hash.GetRange() > h[i]);
    }
    std::sort(h.begin(), h.end());
    BOOST_CHECK(std::adjacent_find(h.begin(), h.end()) == h.end());
}

template<typename Key, typename Generator>
class PerfectHashTester
{
public:
    static void Test(size_t size)
    {
        std::vector<Key> keys;
        Generator::Generate(size, &keys);
        izenelib::am::succinct::PerfectHash<Key> ph;
        BOOST_CHECK_EQUAL(0, ph.Build(keys));
        CheckHashValues(ph, keys);
    }

    static void TestSerialization(size_t size)
    {
        std::vector<Key> keys;
        Generator::Generate(size, &keys);
        std::string data;
        // Save
        {
            izenelib::am::succinct::PerfectHash<Key> mph;
            BOOST_CHECK_EQUAL(0, mph.Build(keys));
            std::ostringstream oss;
            boost::archive::text_oarchive oa(oss);
            oa << mph;
            data = oss.str();
        }
        // Load and test
        {
            std::istringstream iss(data);
            boost::archive::text_iarchive ia(iss);
            izenelib::am::succinct::PerfectHash<Key> mph;
            ia >> mph;
            CheckHashValues(mph, keys);
        }
    }
};

template<typename Key, typename Generator>
class MinimalPerfectHashTester
{
public:
    static void Test(size_t size)
    {
        std::vector<Key> keys;
        Generator::Generate(size, &keys);
        izenelib::am::succinct::MinimalPerfectHash<Key> mph;
        BOOST_CHECK_EQUAL(0, mph.Build(keys));
        CheckHashValues(mph, keys);
        BOOST_CHECK_EQUAL(keys.size(), mph.GetRange());
    }

    static void TestSerialization(size_t size)
    {
        std::vector<Key> keys;
        Generator::Generate(size, &keys);
        std::string data;
        // Save
        {
            izenelib::am::succinct::MinimalPerfectHash<Key> mph;
            BOOST_CHECK_EQUAL(0, mph.Build(keys));
            std::ostringstream oss;
            boost::archive::text_oarchive oa(oss);
            oa << mph;
            data = oss.str();
        }
        // Load and test
        {
            std::istringstream iss(data);
            boost::archive::text_iarchive ia(iss);
            izenelib::am::succinct::MinimalPerfectHash<Key> mph;
            ia >> mph;
            CheckHashValues(mph, keys);
            BOOST_CHECK_EQUAL(keys.size(), mph.GetRange());
        }
    }
};

template<typename T> class RandomElementGenerator;

template<> class RandomElementGenerator<int>
{
public:
    static int Generate()
    {
        return rand();
    }
};

template<typename T, typename U>
class RandomElementGenerator<std::pair<T, U> >
{
public:
    static std::pair<T, U> Generate()
    {
        return std::make_pair(RandomElementGenerator<T>::Generate(),
                              RandomElementGenerator<U>::Generate());
    }
};

template<typename T> class RandomElementGenerator<std::vector<T> >
{
    static constexpr double kLastingProbability = 0.9;
public:
    static std::vector<T> Generate()
    {
        std::vector<T> res;
        while (rand() < RAND_MAX * kLastingProbability)
        {
            res.push_back(RandomElementGenerator<T>::Generate());
        }
        return res;
    }
};

template<> class RandomElementGenerator<std::string>
{
    static constexpr double kLastingProbability = 0.9;
public:
    static std::string Generate()
    {
        std::vector<int> t = RandomElementGenerator<std::vector<int> >::Generate();
        std::string s(t.size(), ' ');
        for (size_t i = 0; i < t.size(); ++i)
        {
            s[i] = 'a' + t[i] % 26;
        }
        return s;
    }
};

template<typename T> class RandomArrayGenerator
{
public:
    static void Generate(size_t s, std::vector<T> *v)
    {
        std::set<T> se;
        while (se.size() < s)
        {
            se.insert(RandomElementGenerator<T>::Generate());
        }
        v->resize(s);
        std::copy(se.begin(), se.end(), v->begin());
    }
};

template<typename T> class OrderedArrayGenerator;

template<> class OrderedArrayGenerator<int>
{
public:
    static void Generate(size_t s, std::vector<int> *v)
    {
        v->resize(s);
        for (size_t i = 0; i < s; ++i)
        {
            v->at(i) = i;
        }
    }
};

template<> class OrderedArrayGenerator<std::pair<int, int> >
{
public:
    static void Generate(size_t s, std::vector<std::pair<int, int> > *v)
    {
        v->clear();
        for (size_t t = 0; ; ++t)
        {
            for (size_t a = 0; a <= t; ++a)
            {
                if (v->size() >= s) return;
                size_t b = t - a;
                v->push_back(std::make_pair(a, b));
            }
        }
    }
};

template<> class OrderedArrayGenerator<std::vector<int> >
{
public:
    static void Generate(size_t s, std::vector<std::vector<int> > *v)
    {
        int base = 2 + rand() % 4;  // [2, 5]

        for (int t = 1; ; ++t)
        {
            std::vector<int> a(t);
            for (;;)
            {
                if (v->size() >= s)
                {
                    return;
                }
                v->push_back(a);
                int c = 1;
                for (int i = t - 1; i >= 0; --i)
                {
                    a[i] += c;
                    c = a[i] / base;
                    a[i] %= base;
                }
                if (c > 0) break;
            }
        }
    }
};

template<> class OrderedArrayGenerator<std::string>
{
public:
    static void Generate(size_t s, std::vector<std::string> *v)
    {
        std::vector<std::vector<int> > tv;
        OrderedArrayGenerator<std::vector<int> >::Generate(s, &tv);
        v->resize(s);
        for (size_t i = 0; i < s; ++i)
        {
            v->at(i).resize(tv[i].size());
            for (size_t j = 0; j < tv[i].size(); ++j)
            {
                v->at(i)[j] = 'a' + (tv[i][j] % 26);
            }
        }
    }
};


typedef boost::mpl::list<// Test |PerfectHash| with basic types
    PerfectHashTester<int, RandomArrayGenerator<int> >,
    PerfectHashTester<int, OrderedArrayGenerator<int> >,
    PerfectHashTester<std::pair<int, int>, RandomArrayGenerator<std::pair<int, int> > >,
    PerfectHashTester<std::pair<int, int>, OrderedArrayGenerator<std::pair<int, int> > >,
    PerfectHashTester<std::string, RandomArrayGenerator<std::string> >,
    PerfectHashTester<std::string, OrderedArrayGenerator<std::string> >,
    PerfectHashTester<std::vector<int>, RandomArrayGenerator<std::vector<int> > >,
    PerfectHashTester<std::vector<int>, OrderedArrayGenerator<std::vector<int> > >,
    // Test |MinimalPerfectHash| with basic types
    MinimalPerfectHashTester<int, RandomArrayGenerator<int> >,
    MinimalPerfectHashTester<int, OrderedArrayGenerator<int> >,
    MinimalPerfectHashTester<std::pair<int, int>, RandomArrayGenerator<std::pair<int, int> > >,
    MinimalPerfectHashTester<std::pair<int, int>, OrderedArrayGenerator<std::pair<int, int> > >,
    MinimalPerfectHashTester<std::string, RandomArrayGenerator<std::string> >,
    MinimalPerfectHashTester<std::string, OrderedArrayGenerator<std::string> >,
    MinimalPerfectHashTester<std::vector<int>, RandomArrayGenerator<std::vector<int> > >,
    MinimalPerfectHashTester<std::vector<int>, OrderedArrayGenerator<std::vector<int> > >,
    // Test with some complex types (heavy...)
    MinimalPerfectHashTester<std::vector<std::string>, RandomArrayGenerator<std::vector<std::string> > >,
    MinimalPerfectHashTester<std::vector<std::pair<int, std::string> >,
    RandomArrayGenerator<std::vector<std::pair<int, std::string> > > >
> HashTesters;

BOOST_AUTO_TEST_SUITE( t_min_perfect_hash_suite )

BOOST_AUTO_TEST_CASE(sample)
{
    // int
    {
        // Prepare the keys
        std::vector<int> t;
        t.push_back(1);
        t.push_back(11);
        t.push_back(111);
        t.push_back(1111);
        t.push_back(11111);
        t.push_back(111111);

        // Build
        izenelib::am::succinct::MinimalPerfectHash<int> mph;
        BOOST_CHECK(0 == mph.Build(t));

        // GetHash
        for (size_t i = 0; i < t.size(); ++i) {
            std::cout << t[i] << "\t" << mph.GetHash(t[i]) << std::endl;
        }
        std::cout << std::endl;
    }

    // string
    {
        // Prepare the keys
        std::vector<std::string> t;
        t.push_back("hoge");
        t.push_back("piyo");
        t.push_back("fuga");
        t.push_back("foo");
        t.push_back("bar");

        // Build
        izenelib::am::succinct::MinimalPerfectHash<std::string> mph;
        BOOST_CHECK(0 == mph.Build(t));

        // GetHash
        for (size_t i = 0; i < t.size(); ++i) {
            std::cout << t[i] << "\t" << mph.GetHash(t[i]) << std::endl;
        }
        std::cout << std::endl;
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(small, T, HashTesters)
{
    const int kNumTrial = 10;
    for (size_t s = 0; s <= 100; ++s)
    {
        for (int t = 0; t < kNumTrial; ++t)
        {
            T::Test(s);
        }
    }
}
/*
BOOST_AUTO_TEST_CASE_TEMPLATE(large, T, HashTesters)
{
    const int kNumTrial = 10;
    const int kMaxSize = 100000;
    for (int t = 0; t < kNumTrial; ++t)
    {
        size_t s = rand() % kMaxSize;
        T::Test(s);
    }
}
*/
BOOST_AUTO_TEST_CASE_TEMPLATE(serialization, T, HashTesters)
{
    const int kNumTrial = 10;
    const int kMaxSize = 100;
    for (int t = 0; t < kNumTrial; ++t)
    {
        size_t s = rand() % kMaxSize;
        T::TestSerialization(s);
    }
}

BOOST_AUTO_TEST_SUITE_END()
