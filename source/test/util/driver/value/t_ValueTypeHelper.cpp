#include <boost/test/unit_test.hpp>

#include <util/driver/value/ValueTypeHelper.h>

using namespace izenelib::driver;

BOOST_AUTO_TEST_SUITE(ValueTypeHelper_test)

BOOST_AUTO_TEST_CASE(TypeCount_test)
{
    BOOST_CHECK(ValueTypeHelper::kTypeCount == 8);
}

BOOST_AUTO_TEST_CASE(ValidType_test)
{
    const ValueTypeHelper& helper = ValueTypeHelper::instance();
    BOOST_CHECK(helper.validType(0));
    BOOST_CHECK(helper.validType(3));
    BOOST_CHECK(helper.validType(7));
}

BOOST_AUTO_TEST_CASE(InvalidType_test)
{
    const ValueTypeHelper& helper = ValueTypeHelper::instance();
    BOOST_CHECK(!helper.validType(-1));
    BOOST_CHECK(!helper.validType(8));
}

BOOST_AUTO_TEST_CASE(type2Name_test)
{
    const ValueTypeHelper& helper = ValueTypeHelper::instance();
    BOOST_CHECK(helper.type2Name(-1) == "Unknown");
    BOOST_CHECK(helper.type2Name(0) == "Null");
    BOOST_CHECK(helper.type2Name(1) == "Int");
    BOOST_CHECK(helper.type2Name(2) == "Uint");
    BOOST_CHECK(helper.type2Name(3) == "Double");
    BOOST_CHECK(helper.type2Name(4) == "String");
    BOOST_CHECK(helper.type2Name(5) == "Bool");
    BOOST_CHECK(helper.type2Name(6) == "Array");
    BOOST_CHECK(helper.type2Name(7) == "Object");
    BOOST_CHECK(helper.type2Name(8) == "Unknown");
}

BOOST_AUTO_TEST_CASE(name2Type_test)
{
    const ValueTypeHelper& helper = ValueTypeHelper::instance();
    BOOST_CHECK(helper.name2Type("Unknown") == 8);
    BOOST_CHECK(helper.name2Type("Null") == 0);
    BOOST_CHECK(helper.name2Type("Int") == 1);
    BOOST_CHECK(helper.name2Type("Uint") == 2);
    BOOST_CHECK(helper.name2Type("Double") == 3);
    BOOST_CHECK(helper.name2Type("String") == 4);
    BOOST_CHECK(helper.name2Type("Bool") == 5);
    BOOST_CHECK(helper.name2Type("Array") == 6);
    BOOST_CHECK(helper.name2Type("Object") == 7);
    BOOST_CHECK(helper.name2Type("object") == 8);
}

BOOST_AUTO_TEST_SUITE_END() // ValueTypeHelper_test
