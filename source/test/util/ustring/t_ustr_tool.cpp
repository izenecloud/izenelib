#include <boost/test/unit_test.hpp>
#include <util/ustring/UString.h>
#include <algorithm>

#define scoped_alloc NULL_ALLOCATOR

using namespace std;
using namespace izenelib::util;
using namespace izenelib::util::ustring_tool;
using namespace boost::unit_test;
using namespace boost;

BOOST_AUTO_TEST_SUITE(ustring_tool_test)

BOOST_AUTO_TEST_CASE(test_basic_Korean_Composer_and_Decomposer)
{
    string beforeProcess("가나다각");
    UString srcUString(beforeProcess, UString::UTF_8);
    UString tarUString, resultUString;

    ustring_tool::processKoreanDecomposer(srcUString, tarUString);
    resultUString += (UCS2Char)4352; // "ㄱ"
    resultUString += (UCS2Char)4449; // "ㅏ"
    resultUString += (UCS2Char)4354; // "ㄴ"
    resultUString += (UCS2Char)4449; // "ㅏ"
    resultUString += (UCS2Char)4355; // "ㄷ"
    resultUString += (UCS2Char)4449; // "ㅏ"
    resultUString += (UCS2Char)4352; // "ㄱ"
    resultUString += (UCS2Char)4449; // "ㅏ"
    resultUString += (UCS2Char)4520; // "ㄱ"

    BOOST_CHECK( tarUString == resultUString );
}

BOOST_AUTO_TEST_CASE(test_Korean_Syllable_Decomposer)
{
    string src("ㄱㅏㄴㅖㄷㅒㅠ"), src2("가녜댸ㅠ");
    UString srcUStr(src, UString::UTF_8), src2UStr(src2, UString::UTF_8);
    UString decUStr, dec2UStr;
    BOOST_CHECK( processKoreanDecomposerWithCharacterCheck(srcUStr,   decUStr) );
    BOOST_CHECK( processKoreanDecomposerWithCharacterCheck(src2UStr, dec2UStr) );
    BOOST_CHECK( decUStr == dec2UStr );
}

BOOST_AUTO_TEST_CASE(test_return_value_of_Korean_Composer_and_Decomposer)
{
    UString test("운수좋은날izenesoft問子あ123도", UString::UTF_8);
    UString test2, test3;
    BOOST_CHECK( processKoreanDecomposerWithCharacterCheck(test, test2) );
    BOOST_CHECK( processKoreanComposer(test2, test3) );
    BOOST_CHECK( test == test3 );
}

BOOST_AUTO_TEST_CASE(test_return_value_of_Korean_Composer_and_Decomposer2)
{
    UString test("izenesoft問子あ123", UString::UTF_8);
    UString test2, test3;
    BOOST_CHECK( !processKoreanDecomposerWithCharacterCheck(test, test2) );
    BOOST_CHECK( !processKoreanComposer(test2, test3) );
    BOOST_CHECK( test == test3 );
}

BOOST_AUTO_TEST_SUITE_END()

#undef scoped_alloc
