#include <boost/test/unit_test.hpp>
#include <util/ustring/UString.h>


#define scoped_alloc NULL_ALLOCATOR

#define USTRING_CONVERT_ENCODING_TYPE_TEST_CASE(name, str, enc)         \
BOOST_AUTO_TEST_CASE(name##_test)                                       \
{                                                                       \
    BOOST_CHECK_EQUAL(UString::enc, UString::convertEncodingTypeFromStringToEnum(str)); \
}

using namespace std;
using namespace izenelib::util;;
using namespace boost::unit_test;
using namespace boost;

BOOST_AUTO_TEST_SUITE(ConvertEncoding_TestSuite)

USTRING_CONVERT_ENCODING_TYPE_TEST_CASE(test_UTF_8, "UTF-8", UTF_8)
USTRING_CONVERT_ENCODING_TYPE_TEST_CASE(test_EUC_KR, "EUC-KR", EUC_KR)
USTRING_CONVERT_ENCODING_TYPE_TEST_CASE(test_CP949, "CP949", CP949)
USTRING_CONVERT_ENCODING_TYPE_TEST_CASE(test_EUC_JP, "EUC-JP", EUC_JP)
USTRING_CONVERT_ENCODING_TYPE_TEST_CASE(test_SJIS, "SJIS", SJIS)
USTRING_CONVERT_ENCODING_TYPE_TEST_CASE(test_GB2312, "GB2312", GB2312)
USTRING_CONVERT_ENCODING_TYPE_TEST_CASE(test_BIG5, "BIG-5", BIG5)
USTRING_CONVERT_ENCODING_TYPE_TEST_CASE(test_ISO8859_15, "ISO8859-15", ISO8859_15)

USTRING_CONVERT_ENCODING_TYPE_TEST_CASE(test_utf_8, "utf-8", UTF_8)
USTRING_CONVERT_ENCODING_TYPE_TEST_CASE(test_euc_kr, "euc-kr", EUC_KR)
USTRING_CONVERT_ENCODING_TYPE_TEST_CASE(test_cp949, "cp949", CP949)
USTRING_CONVERT_ENCODING_TYPE_TEST_CASE(test_euc_jp, "euc-jp", EUC_JP)
USTRING_CONVERT_ENCODING_TYPE_TEST_CASE(test_sjis, "sjis", SJIS)
USTRING_CONVERT_ENCODING_TYPE_TEST_CASE(test_gb2312, "gb2312", GB2312)
USTRING_CONVERT_ENCODING_TYPE_TEST_CASE(test_big5, "big-5", BIG5)
USTRING_CONVERT_ENCODING_TYPE_TEST_CASE(test_iso8859_15, "iso8859-15", ISO8859_15)



BOOST_AUTO_TEST_SUITE_END() // BasicInterface_TestSuite

#undef USTRING_CONVERT_ENCODING_TYPE_TEST_CASE
#undef scoped_alloc
