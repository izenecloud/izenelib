#include <boost/test/unit_test.hpp>

#include <string>
#include <util/string/RangeTokenizer.h>
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
    typedef RangeTokenizer<tokenizer_func, iterator> tokenizer;
    typedef tokenizer::iterator token_iterator;
};

} // namespace {annonymous}

BOOST_FIXTURE_TEST_SUITE(RangeTokenIterator_test, FirstNCharTokenizerFuncFixture)

BOOST_AUTO_TEST_CASE(RangeConstructor_test)
{
    std::string str("123");
    tokenizer tokens(str.begin(), str.end());

    token_iterator it = tokens.begin();
    token_iterator end = tokens.end();

    BOOST_CHECK(it != end);
    BOOST_CHECK(it->first == str.begin() + 0);
    BOOST_CHECK(it->second == str.begin() + 1);

    ++it;
    BOOST_CHECK(it != end);
    BOOST_CHECK(it->first == str.begin() + 1);
    BOOST_CHECK(it->second == str.begin() + 2);

    ++it;
    BOOST_CHECK(it != end);
    BOOST_CHECK(it->first == str.begin() + 2);
    BOOST_CHECK(it->second == str.begin() + 3);

    ++it;
    BOOST_CHECK(it == end);
}

BOOST_AUTO_TEST_CASE(ContainerConstructor_test)
{
    std::string str("123");
    tokenizer tokens(str);

    token_iterator it = tokens.begin();
    token_iterator end = tokens.end();

    BOOST_CHECK(it != end);
    BOOST_CHECK(it->first == str.begin() + 0);
    BOOST_CHECK(it->second == str.begin() + 1);

    ++it;
    BOOST_CHECK(it != end);
    BOOST_CHECK(it->first == str.begin() + 1);
    BOOST_CHECK(it->second == str.begin() + 2);

    ++it;
    BOOST_CHECK(it != end);
    BOOST_CHECK(it->first == str.begin() + 2);
    BOOST_CHECK(it->second == str.begin() + 3);

    ++it;
    BOOST_CHECK(it == end);
}

BOOST_AUTO_TEST_CASE(RangeFuncConstructor_test)
{
    std::string str("123");
    tokenizer_func func(1);
    tokenizer tokens(str.begin(), str.end(), func);

    token_iterator it = tokens.begin();
    token_iterator end = tokens.end();

    BOOST_CHECK(it != end);
    BOOST_CHECK(it->first == str.begin() + 0);
    BOOST_CHECK(it->second == str.begin() + 1);

    ++it;
    BOOST_CHECK(it == end);
}

BOOST_AUTO_TEST_CASE(ContainerFuncConstructor_test)
{
    std::string str("123");
    tokenizer_func func(1);
    tokenizer tokens(str, func);

    token_iterator it = tokens.begin();
    token_iterator end = tokens.end();

    BOOST_CHECK(it != end);
    BOOST_CHECK(it->first == str.begin() + 0);
    BOOST_CHECK(it->second == str.begin() + 1);

    ++it;
    BOOST_CHECK(it == end);
}

BOOST_AUTO_TEST_SUITE_END() // RangeTokenIterator_test
