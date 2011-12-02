#include <boost/test/unit_test.hpp>

#include <string>
#include <util/string/RangeTokenIterator.h>
#include <iostream>

using namespace izenelib::util;

namespace { // {anonymous}

// test func to return first n char as tokenizer
struct FirstNCharTokenizerFunc
{
    // 0 is unlimited
    explicit FirstNCharTokenizerFunc(unsigned n = 0)
    : count_(0), n_(n)
    {}

    template<typename Iterator>
    bool operator()(Iterator& next,
                    Iterator end,
                    std::pair<Iterator, Iterator>& tok)
    {
        if (next == end || (n_ > 0 && count_ >= n_))
        {
            return false;
        }

        tok.first = next;
        tok.second = ++next;
        ++count_;
        return true;
    }

    void reset()
    {
        count_ = 0;
    }

private:
    unsigned count_;
    unsigned n_;
};

struct FirstNCharTokenizerFuncFixture
{
    typedef FirstNCharTokenizerFunc tokenizer_func;
    typedef std::string::iterator iterator;
    typedef RangeTokenIterator<tokenizer_func, iterator> token_iterator;
};

} // namespace {annonymous}

BOOST_FIXTURE_TEST_SUITE(RangeTokenIterator_test, FirstNCharTokenizerFuncFixture)

BOOST_AUTO_TEST_CASE(EmptyConstructor_test)
{
    token_iterator it;
    BOOST_CHECK(it.atEnd());
}

BOOST_AUTO_TEST_CASE(EmptyConstructorEqual_test)
{
    token_iterator it1;
    token_iterator it2;

    BOOST_CHECK(it1 == it2);
}

BOOST_AUTO_TEST_CASE(EmptyRange_test)
{
    std::string empty;
    token_iterator it(empty.begin(), empty.end());

    BOOST_CHECK(it == token_iterator());
    BOOST_CHECK(it.atEnd());
}

BOOST_AUTO_TEST_CASE(GetToken_test)
{
    std::string str("123");
    token_iterator it(str.begin(), str.end());
    token_iterator end1(str.end(), str.end());
    token_iterator end2;

    BOOST_CHECK(it != end1);
    BOOST_CHECK(it != end2);
    BOOST_CHECK(it->first == str.begin() + 0);
    BOOST_CHECK(it->second == str.begin() + 1);

    ++it;
    BOOST_CHECK(it != end1);
    BOOST_CHECK(it != end2);
    BOOST_CHECK(it->first == str.begin() + 1);
    BOOST_CHECK(it->second == str.begin() + 2);

    ++it;
    BOOST_CHECK(it != end1);
    BOOST_CHECK(it != end2);
    BOOST_CHECK(it->first == str.begin() + 2);
    BOOST_CHECK(it->second == str.begin() + 3);

    ++it;
    BOOST_CHECK(it == end1);
    BOOST_CHECK(it == end2);
}

BOOST_AUTO_TEST_CASE(NoTokenFound_test)
{
    std::string str("123");
    tokenizer_func func(1);
    token_iterator it(func, str.begin(), str.end());
    token_iterator end1(func, str.end(), str.end());
    token_iterator end2;

    ++it;
    BOOST_CHECK(it == end1);
    BOOST_CHECK(it == end2);
}

BOOST_AUTO_TEST_SUITE_END() // RangeTokenIterator_test
