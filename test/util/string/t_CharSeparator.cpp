#include <boost/test/unit_test.hpp>

#include <string>
#include <util/string/RangeTokenizer.h>
#include <util/string/CharSeparator.h>

using namespace izenelib::util;

namespace { // {anonymous}

struct CharSeparatorFixture
{
    typedef CharSeparator<char> tokenizer_func;
    typedef std::string string;
    typedef string::iterator iterator;
    typedef RangeTokenizer<tokenizer_func, iterator> tokenizer_t;

    tokenizer_func createFunc()
    {
        return tokenizer_func(separator.begin(), separator.end());
    }

    tokenizer_t createTokenizer()
    {
        return tokenizer_t(str, createFunc());
    }

    string separator;
    string str;
};

}

BOOST_FIXTURE_TEST_SUITE(CharSeparator_test, CharSeparatorFixture)

BOOST_AUTO_TEST_CASE(EmptyStr_test)
{
    tokenizer_t tokenizer(createTokenizer());

    BOOST_CHECK(tokenizer.begin() == tokenizer.end());
}

BOOST_AUTO_TEST_CASE(EmptySeparator_test)
{
    str = "a,b";
    tokenizer_t tokenizer(createTokenizer());

    tokenizer_t::iterator it = tokenizer.begin();
    tokenizer_t::iterator itEnd = tokenizer.end();
    BOOST_CHECK(it != itEnd);
    BOOST_CHECK(it->first == str.begin());
    BOOST_CHECK(it->second == str.end());

    ++it;
    BOOST_CHECK(it == itEnd);
}

BOOST_AUTO_TEST_CASE(SingleSeparator_test)
{
    separator = ",";
    str = "a,b";

    tokenizer_t tokenizer(createTokenizer());

    tokenizer_t::iterator it = tokenizer.begin();
    tokenizer_t::iterator itEnd = tokenizer.end();
    string tok;

    BOOST_CHECK(it != itEnd);
    tok.assign(it->first, it->second);
    BOOST_CHECK(tok == "a");

    ++it;
    tok.assign(it->first, it->second);
    BOOST_CHECK(tok == "b");

    ++it;
    BOOST_CHECK(it == itEnd);
}

BOOST_AUTO_TEST_CASE(SingleSeparatorNotExist_test)
{
    separator = ",";
    str = "a.b";

    tokenizer_t tokenizer(createTokenizer());

    tokenizer_t::iterator it = tokenizer.begin();
    tokenizer_t::iterator itEnd = tokenizer.end();
    string tok;

    BOOST_CHECK(it != itEnd);
    tok.assign(it->first, it->second);
    BOOST_CHECK(tok == "a.b");

    ++it;
    BOOST_CHECK(it == itEnd);
}

BOOST_AUTO_TEST_CASE(TwoSeperators_test)
{
    separator = ".,";
    str = "a.b,c";

    tokenizer_t tokenizer(createTokenizer());

    tokenizer_t::iterator it = tokenizer.begin();
    tokenizer_t::iterator itEnd = tokenizer.end();
    string tok;

    BOOST_CHECK(it != itEnd);
    tok.assign(it->first, it->second);
    BOOST_CHECK(tok == "a");

    ++it;
    BOOST_CHECK(it != itEnd);
    tok.assign(it->first, it->second);
    BOOST_CHECK(tok == "b");

    ++it;
    BOOST_CHECK(it != itEnd);
    tok.assign(it->first, it->second);
    BOOST_CHECK(tok == "c");

    ++it;
    BOOST_CHECK(it == itEnd);
}

BOOST_AUTO_TEST_CASE(FirstCharIsSeparator_test)
{
    separator = ",";
    str = ",c";

    tokenizer_t tokenizer(createTokenizer());

    tokenizer_t::iterator it = tokenizer.begin();
    tokenizer_t::iterator itEnd = tokenizer.end();
    string tok;

    BOOST_CHECK(it != itEnd);
    BOOST_CHECK(it->first == str.begin());
    BOOST_CHECK(it->second == it->first);

    ++it;
    BOOST_CHECK(it != itEnd);
    tok.assign(it->first, it->second);
    BOOST_CHECK(tok == "c");

    ++it;
    BOOST_CHECK(it == itEnd);
}

BOOST_AUTO_TEST_CASE(LastCharIsSeparator_test)
{
    separator = ",";
    str = "c,";

    tokenizer_t tokenizer(createTokenizer());

    tokenizer_t::iterator it = tokenizer.begin();
    tokenizer_t::iterator itEnd = tokenizer.end();
    string tok;

    BOOST_CHECK(it != itEnd);
    tok.assign(it->first, it->second);
    BOOST_CHECK(tok == "c");

    ++it;
    BOOST_CHECK(it != itEnd);
    BOOST_CHECK(it->first == str.end());
    BOOST_CHECK(it->second == it->first);

    ++it;
    BOOST_CHECK(it == itEnd);
}

BOOST_AUTO_TEST_CASE(ConsecutiveSeparators_test)
{
    separator = ",";
    str = "c,,d";

    tokenizer_t tokenizer(createTokenizer());

    tokenizer_t::iterator it = tokenizer.begin();
    tokenizer_t::iterator itEnd = tokenizer.end();
    string tok;

    BOOST_CHECK(it != itEnd);
    tok.assign(it->first, it->second);
    BOOST_CHECK(tok == "c");

    ++it;
    BOOST_CHECK(it != itEnd);
    BOOST_CHECK(it->first == it->second);

    ++it;
    BOOST_CHECK(it != itEnd);
    tok.assign(it->first, it->second);
    BOOST_CHECK(tok == "d");

    ++it;
    BOOST_CHECK(it == itEnd);
}

BOOST_AUTO_TEST_SUITE_END() // CharSeparator_test
