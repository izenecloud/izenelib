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

    std::size_t size(const Value& v) const
    {
        return v.size();
    }
};

} // namespace {anonymous}

BOOST_FIXTURE_TEST_SUITE(Size_test, ValueFixture)

BOOST_AUTO_TEST_CASE(NullValue_test)
{
    BOOST_CHECK(0 == size(nullValue));
}

BOOST_AUTO_TEST_CASE(Int_test)
{
    BOOST_CHECK(1 == size(Value(1)));
}

BOOST_AUTO_TEST_CASE(Uint_test)
{
    BOOST_CHECK(1 == size(Value(1U)));
}

BOOST_AUTO_TEST_CASE(Bool_test)
{
    BOOST_CHECK(1 == size(Value(true)));
}

BOOST_AUTO_TEST_CASE(Double_test)
{
    BOOST_CHECK(1 == size(Value(1.1)));
}

BOOST_AUTO_TEST_CASE(Str_test)
{
    BOOST_CHECK(1 == size(Value("abcdef")));
}

BOOST_AUTO_TEST_CASE(Array_test)
{
    BOOST_CHECK(1 == size(arrayValue));
    arrayValue() = 2;
    BOOST_CHECK(2 == size(arrayValue));
    arrayValue.clear();
    BOOST_CHECK(0 == size(arrayValue));
}

BOOST_AUTO_TEST_CASE(Object_test)
{
    BOOST_CHECK(1 == size(objectValue));
    objectValue["test"] = 10;
    BOOST_CHECK(1 == size(objectValue));
    objectValue["tese"] = 10;
    BOOST_CHECK(2 == size(objectValue));
    objectValue.clear();
    BOOST_CHECK(0 == size(objectValue));
}

BOOST_AUTO_TEST_SUITE_END() // Set_test
