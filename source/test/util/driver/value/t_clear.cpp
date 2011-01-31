#include <boost/test/unit_test.hpp>

#include <util/driver/value/Value.h>

using namespace izenelib::driver;

namespace { // {anonymous}
struct ValueFixture
{
    Value arrayValue;
    Value objectValue;

    ValueFixture()
    {
        Value::ArrayType array;
        array.push_back(1);
        arrayValue = array;

        Value::ObjectType object;
        object["test"] = 1;
        objectValue = object;
    }
};

} // namespace {anonymous}

BOOST_FIXTURE_TEST_SUITE(Clear_test, ValueFixture)

BOOST_AUTO_TEST_CASE(NullValue_test)
{
    Value v;
    v.clear();
    BOOST_CHECK(v == nullValue);
}

BOOST_AUTO_TEST_CASE(Int_test)
{
    Value v(1);
    v.clear();
    BOOST_CHECK(0 == v);
}

BOOST_AUTO_TEST_CASE(Uint_test)
{
    Value v(1U);
    v.clear();
    BOOST_CHECK(0U == v);
}

BOOST_AUTO_TEST_CASE(Bool_test)
{
    Value v(true);
    v.clear();
    BOOST_CHECK(false == v);
}

BOOST_AUTO_TEST_CASE(Str_test)
{
    Value v("abcdef");
    v.clear();
    BOOST_CHECK("" == v);
}

BOOST_AUTO_TEST_CASE(Array_test)
{
    arrayValue.clear();
    BOOST_CHECK(arrayValue.size() == 0);
    BOOST_CHECK(arrayValue.type() == Value::kArrayType);
}

BOOST_AUTO_TEST_CASE(Object_test)
{
    objectValue.clear();
    BOOST_CHECK(objectValue.size() == 0);
    BOOST_CHECK(objectValue.type() == Value::kObjectType);
}

BOOST_AUTO_TEST_SUITE_END() // Set_test
