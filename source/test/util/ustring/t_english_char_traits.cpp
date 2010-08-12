#include <boost/test/unit_test.hpp>
#include <util/ustring/UString.h>
#include <algorithm>
#include <iostream>

#define scoped_alloc NULL_ALLOCATOR

using namespace std;
using namespace izenelib::util;
using namespace boost::unit_test;
using namespace boost;

BOOST_AUTO_TEST_SUITE(CharTraits_test)

BOOST_AUTO_TEST_CASE(GraphChar_test)
{
    UString ustr("1a!\1 ", UString::ISO8859_15);

    BOOST_CHECK(ustr.isGraphChar(0));
    BOOST_CHECK(ustr.isGraphChar(1));
    BOOST_CHECK(ustr.isGraphChar(2));
    BOOST_CHECK(!ustr.isGraphChar(3));
    BOOST_CHECK(!ustr.isGraphChar(4));

    BOOST_CHECK(UString::isThisGraphChar(ustr[0]));
    BOOST_CHECK(UString::isThisGraphChar(ustr[1]));
    BOOST_CHECK(UString::isThisGraphChar(ustr[2]));
    BOOST_CHECK(!UString::isThisGraphChar(ustr[3]));
    BOOST_CHECK(!UString::isThisGraphChar(ustr[4]));
}

BOOST_AUTO_TEST_CASE(SpaceChar_test)
{
    // 1, 2, ASCII space, new line
    UString ustr("1a! \n", UString::UTF_8);

    BOOST_CHECK(!ustr.isSpaceChar(0));
    BOOST_CHECK(!ustr.isSpaceChar(1));
    BOOST_CHECK(!ustr.isSpaceChar(2));
    BOOST_CHECK(ustr.isSpaceChar(3));
    BOOST_CHECK(ustr.isSpaceChar(4));

    BOOST_CHECK(!UString::isThisSpaceChar(ustr[0]));
    BOOST_CHECK(!UString::isThisSpaceChar(ustr[1]));
    BOOST_CHECK(!UString::isThisSpaceChar(ustr[2]));
    BOOST_CHECK(UString::isThisSpaceChar(ustr[3]));
    BOOST_CHECK(UString::isThisSpaceChar(ustr[4]));
}

BOOST_AUTO_TEST_CASE(PunctuationChar_test)
{
    UString ustr("1a!\1 ", UString::ISO8859_15);

    BOOST_CHECK(!ustr.isPunctuationChar(0));
    BOOST_CHECK(!ustr.isPunctuationChar(1));
    BOOST_CHECK(ustr.isPunctuationChar(2));
    BOOST_CHECK(!ustr.isPunctuationChar(3));
    BOOST_CHECK(!ustr.isPunctuationChar(4));

    BOOST_CHECK(!UString::isThisPunctuationChar(ustr[0]));
    BOOST_CHECK(!UString::isThisPunctuationChar(ustr[1]));
    BOOST_CHECK(UString::isThisPunctuationChar(ustr[2]));
    BOOST_CHECK(!UString::isThisPunctuationChar(ustr[3]));
    BOOST_CHECK(!UString::isThisPunctuationChar(ustr[4]));
}

BOOST_AUTO_TEST_CASE(LanguageChar_test)
{
    UString ustr("1a! ", UString::UTF_8);
    BOOST_CHECK(!ustr.isLanguageChar(0));
    BOOST_CHECK(ustr.isLanguageChar(1));
    BOOST_CHECK(!ustr.isLanguageChar(2));
    BOOST_CHECK(!ustr.isLanguageChar(3));

    BOOST_CHECK(!UString::isThisLanguageChar(ustr[0]));
    BOOST_CHECK(UString::isThisLanguageChar(ustr[1]));
    BOOST_CHECK(!UString::isThisLanguageChar(ustr[2]));
    BOOST_CHECK(!UString::isThisLanguageChar(ustr[3]));
}

BOOST_AUTO_TEST_CASE(UpperChar_test)
{
    UString ustr("`az{@AZ[", UString::UTF_8);
    BOOST_CHECK(!ustr.isUpperChar(0)); // `
    BOOST_CHECK(!ustr.isUpperChar(1)); // a
    BOOST_CHECK(!ustr.isUpperChar(2)); // z
    BOOST_CHECK(!ustr.isUpperChar(3)); // {
    BOOST_CHECK(!ustr.isUpperChar(4)); // @
    BOOST_CHECK(ustr.isUpperChar(5)); // A
    BOOST_CHECK(ustr.isUpperChar(6)); // Z
    BOOST_CHECK(!ustr.isUpperChar(7)); // [

    BOOST_CHECK(!UString::isThisUpperChar(ustr[0])); // `
    BOOST_CHECK(!UString::isThisUpperChar(ustr[1])); // a
    BOOST_CHECK(!UString::isThisUpperChar(ustr[2])); // z
    BOOST_CHECK(!UString::isThisUpperChar(ustr[3])); // {
    BOOST_CHECK(!UString::isThisUpperChar(ustr[4])); // @
    BOOST_CHECK(UString::isThisUpperChar(ustr[5])); // A
    BOOST_CHECK(UString::isThisUpperChar(ustr[6])); // Z
    BOOST_CHECK(!UString::isThisUpperChar(ustr[7])); // [
}

BOOST_AUTO_TEST_CASE(LowerChar_test)
{
    UString ustr("`az{@AZ[", UString::UTF_8);
    BOOST_CHECK(!ustr.isLowerChar(0)); // `
    BOOST_CHECK(ustr.isLowerChar(1)); // a
    BOOST_CHECK(ustr.isLowerChar(2)); // z
    BOOST_CHECK(!ustr.isLowerChar(3)); // {
    BOOST_CHECK(!ustr.isLowerChar(4)); // @
    BOOST_CHECK(!ustr.isLowerChar(5)); // A
    BOOST_CHECK(!ustr.isLowerChar(6)); // Z
    BOOST_CHECK(!ustr.isLowerChar(7)); // [

    BOOST_CHECK(!UString::isThisLowerChar(ustr[0])); // `
    BOOST_CHECK(UString::isThisLowerChar(ustr[1])); // a
    BOOST_CHECK(UString::isThisLowerChar(ustr[2])); // z
    BOOST_CHECK(!UString::isThisLowerChar(ustr[3])); // {
    BOOST_CHECK(!UString::isThisLowerChar(ustr[4])); // @
    BOOST_CHECK(!UString::isThisLowerChar(ustr[5])); // A
    BOOST_CHECK(!UString::isThisLowerChar(ustr[6])); // Z
    BOOST_CHECK(!UString::isThisLowerChar(ustr[7])); // [
}

BOOST_AUTO_TEST_CASE(DigitChar_test)
{
    UString ustr("/09:", UString::UTF_8);
    BOOST_CHECK(!ustr.isDigitChar(0)); // /
    BOOST_CHECK(ustr.isDigitChar(1)); // 0
    BOOST_CHECK(ustr.isDigitChar(2)); // 9
    BOOST_CHECK(!ustr.isDigitChar(3)); // :

    BOOST_CHECK(!UString::isThisDigitChar(ustr[0])); // /
    BOOST_CHECK(UString::isThisDigitChar(ustr[1])); // 0
    BOOST_CHECK(UString::isThisDigitChar(ustr[2])); // 9
    BOOST_CHECK(!UString::isThisDigitChar(ustr[3])); // :
}

BOOST_AUTO_TEST_CASE(AlnumChar_test)
{
    UString ustr("/09:`az{@AZ[", UString::UTF_8);

    BOOST_CHECK(!ustr.isAlnumChar(0)); // /
    BOOST_CHECK(ustr.isAlnumChar(1)); // 0
    BOOST_CHECK(ustr.isAlnumChar(2)); // 9
    BOOST_CHECK(!ustr.isAlnumChar(3)); // :
    BOOST_CHECK(!ustr.isAlnumChar(4)); // `
    BOOST_CHECK(ustr.isAlnumChar(5)); // a
    BOOST_CHECK(ustr.isAlnumChar(6)); // z
    BOOST_CHECK(!ustr.isAlnumChar(7)); // {
    BOOST_CHECK(!ustr.isAlnumChar(8)); // @
    BOOST_CHECK(ustr.isAlnumChar(9)); // A
    BOOST_CHECK(ustr.isAlnumChar(10)); // Z
    BOOST_CHECK(!ustr.isAlnumChar(11)); // [

    BOOST_CHECK(!UString::isThisAlnumChar(ustr[0])); // /
    BOOST_CHECK(UString::isThisAlnumChar(ustr[1])); // 0
    BOOST_CHECK(UString::isThisAlnumChar(ustr[2])); // 9
    BOOST_CHECK(!UString::isThisAlnumChar(ustr[3])); // :
    BOOST_CHECK(!UString::isThisAlnumChar(ustr[4])); // `
    BOOST_CHECK(UString::isThisAlnumChar(ustr[5])); // a
    BOOST_CHECK(UString::isThisAlnumChar(ustr[6])); // z
    BOOST_CHECK(!UString::isThisAlnumChar(ustr[7])); // {
    BOOST_CHECK(!UString::isThisAlnumChar(ustr[8])); // @
    BOOST_CHECK(UString::isThisAlnumChar(ustr[9])); // A
    BOOST_CHECK(UString::isThisAlnumChar(ustr[10])); // Z
    BOOST_CHECK(!UString::isThisAlnumChar(ustr[11])); // [
}

BOOST_AUTO_TEST_CASE(AlnumCharType_test)
{
    UString ustr("/09:`az{@AZ[", UString::UTF_8);
    BOOST_CHECK_EQUAL(UCS2_UNDEF, ustr.charType(0)); // /
    BOOST_CHECK_EQUAL(UCS2_DIGIT, ustr.charType(1)); // 0
    BOOST_CHECK_EQUAL(UCS2_DIGIT, ustr.charType(2)); // 9
    BOOST_CHECK_EQUAL(UCS2_UNDEF, ustr.charType(3)); // :
    BOOST_CHECK_EQUAL(UCS2_UNDEF, ustr.charType(4)); // `
    BOOST_CHECK_EQUAL(UCS2_ALPHA, ustr.charType(5)); // a
    BOOST_CHECK_EQUAL(UCS2_ALPHA, ustr.charType(6)); // z
    BOOST_CHECK_EQUAL(UCS2_UNDEF, ustr.charType(7)); // {
    BOOST_CHECK_EQUAL(UCS2_UNDEF, ustr.charType(8)); // @
    BOOST_CHECK_EQUAL(UCS2_ALPHA, ustr.charType(9)); // A
    BOOST_CHECK_EQUAL(UCS2_ALPHA, ustr.charType(10)); // Z
    BOOST_CHECK_EQUAL(UCS2_UNDEF, ustr.charType(11)); // [
}

BOOST_AUTO_TEST_CASE(XdigitChar_test)
{
    UString ustr("/09:`afg@AFG", UString::UTF_8);
    BOOST_CHECK(!ustr.isXdigitChar(0)); // /
    BOOST_CHECK(ustr.isXdigitChar(1)); // 0
    BOOST_CHECK(ustr.isXdigitChar(2)); // 9
    BOOST_CHECK(!ustr.isXdigitChar(3)); // :
    BOOST_CHECK(!ustr.isXdigitChar(4)); // `
    BOOST_CHECK(ustr.isXdigitChar(5)); // a
    BOOST_CHECK(ustr.isXdigitChar(6)); // z
    BOOST_CHECK(!ustr.isXdigitChar(7)); // g
    BOOST_CHECK(!ustr.isXdigitChar(8)); // @
    BOOST_CHECK(ustr.isXdigitChar(9)); // A
    BOOST_CHECK(ustr.isXdigitChar(10)); // Z
    BOOST_CHECK(!ustr.isXdigitChar(11)); // G

    BOOST_CHECK(!UString::isThisXdigitChar(ustr[0])); // /
    BOOST_CHECK(UString::isThisXdigitChar(ustr[1])); // 0
    BOOST_CHECK(UString::isThisXdigitChar(ustr[2])); // 9
    BOOST_CHECK(!UString::isThisXdigitChar(ustr[3])); // :
    BOOST_CHECK(!UString::isThisXdigitChar(ustr[4])); // `
    BOOST_CHECK(UString::isThisXdigitChar(ustr[5])); // a
    BOOST_CHECK(UString::isThisXdigitChar(ustr[6])); // z
    BOOST_CHECK(!UString::isThisXdigitChar(ustr[7])); // g
    BOOST_CHECK(!UString::isThisXdigitChar(ustr[8])); // @
    BOOST_CHECK(UString::isThisXdigitChar(ustr[9])); // A
    BOOST_CHECK(UString::isThisXdigitChar(ustr[10])); // Z
    BOOST_CHECK(!UString::isThisXdigitChar(ustr[11])); // G
}

BOOST_AUTO_TEST_CASE(ToLower_test)
{
    UString ustr("/09:`az{@AZ[", UString::UTF_8);
    UString expect("/09:`az{@az[", UString::UTF_8);

    ustr.toLowerString();

    BOOST_CHECK(ustr == expect);
}

BOOST_AUTO_TEST_CASE(ToUpper_test)
{
    UString ustr("/09:`az{@AZ[", UString::UTF_8);
    UString expect("/09:`AZ{@AZ[", UString::UTF_8);

    ustr.toUpperString();

    BOOST_CHECK(ustr == expect);
}


BOOST_AUTO_TEST_SUITE_END() // CharTraits_test
