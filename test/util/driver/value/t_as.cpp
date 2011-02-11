#include <boost/test/unit_test.hpp>

#include <util/driver/value/Value.h>
#include <util/driver/value/as.h>

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

#define IZENELIB_DRIVER_CHECK_AS(type, expect, value)   \
    BOOST_CHECK(expect == as##type(value)); \
    BOOST_CHECK(expect == as<Value::type##Type>(value))

BOOST_FIXTURE_TEST_SUITE(As_test, ValueFixture)

BOOST_AUTO_TEST_CASE(NullAsInt_test)
{
    IZENELIB_DRIVER_CHECK_AS(Int, 0, nullValue);
}

BOOST_AUTO_TEST_CASE(BoolAsInt_test)
{
    IZENELIB_DRIVER_CHECK_AS(Int, 0, false);
    IZENELIB_DRIVER_CHECK_AS(Int, 1, true);
}

BOOST_AUTO_TEST_CASE(UintAsInt_test)
{
    IZENELIB_DRIVER_CHECK_AS(Int, 0, 0U);
    IZENELIB_DRIVER_CHECK_AS(Int, 1, 1U);
}

BOOST_AUTO_TEST_CASE(DoubleAsInt_test)
{
    IZENELIB_DRIVER_CHECK_AS(Int, 1, 1.1);
}

BOOST_AUTO_TEST_CASE(StrAsInt_test)
{
    IZENELIB_DRIVER_CHECK_AS(Int, 123, "123");
}

BOOST_AUTO_TEST_CASE(InvalidStrAsInt_test)
{
    IZENELIB_DRIVER_CHECK_AS(Int, 0, "abc");
}

BOOST_AUTO_TEST_CASE(IntAsBool_test)
{
    IZENELIB_DRIVER_CHECK_AS(Bool, false, 0);
    IZENELIB_DRIVER_CHECK_AS(Bool, true, 1);
    IZENELIB_DRIVER_CHECK_AS(Bool, true, -1);
}

BOOST_AUTO_TEST_CASE(IntAsString_test)
{
    IZENELIB_DRIVER_CHECK_AS(String, "0", 0);
    IZENELIB_DRIVER_CHECK_AS(String, "1", 1);
    IZENELIB_DRIVER_CHECK_AS(String, "-1", -1);
}

BOOST_AUTO_TEST_CASE(ArrayAsInt_test)
{
    IZENELIB_DRIVER_CHECK_AS(Int, 0, arrayValue);
}

BOOST_AUTO_TEST_CASE(ObjectAsInt_test)
{
    IZENELIB_DRIVER_CHECK_AS(Int, 0, objectValue);
}

BOOST_AUTO_TEST_CASE(StrAsBool_test)
{
    IZENELIB_DRIVER_CHECK_AS(Bool, true, "x");
    IZENELIB_DRIVER_CHECK_AS(Bool, true, "N");
    IZENELIB_DRIVER_CHECK_AS(Bool, true, "no");
    IZENELIB_DRIVER_CHECK_AS(Bool, true, "n");
    IZENELIB_DRIVER_CHECK_AS(Bool, false, "");
}

BOOST_AUTO_TEST_CASE(BoolAsTring_test)
{
    IZENELIB_DRIVER_CHECK_AS(String, "0", false);
    IZENELIB_DRIVER_CHECK_AS(String, "1", true);
}

BOOST_AUTO_TEST_CASE(ArrayAsString_test)
{
    IZENELIB_DRIVER_CHECK_AS(String, "", arrayValue);
}

BOOST_AUTO_TEST_CASE(ObjectAsString_test)
{
    IZENELIB_DRIVER_CHECK_AS(String, "", objectValue);
}

BOOST_AUTO_TEST_CASE(NegativeIntAsDouble_test)
{
    IZENELIB_DRIVER_CHECK_AS(Double, -1.0, -1);
}

BOOST_AUTO_TEST_CASE(NegativeIntAsUint_test)
{
    IZENELIB_DRIVER_CHECK_AS(Uint, 0, -1);
}

BOOST_AUTO_TEST_SUITE_END() // As_test

#undef IZENELIB_DRIVER_CHECK_AS
