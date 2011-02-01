#include <boost/test/unit_test.hpp>

#include <util/driver/value/Value.h>

using namespace izenelib::driver;

namespace { // {anonymous}
template<typename T1, typename T2>
inline bool lessXorGreater(const T1& t1, const T2& t2)
{
    return (t1 < t2) ^ (t2 < t1);
}

} // namespace {anonymous}

BOOST_AUTO_TEST_SUITE(Compare_test)

BOOST_AUTO_TEST_CASE(Equal_test)
{
    BOOST_CHECK(Value(1) == Value(1));
    BOOST_CHECK(Value(1U) == Value(1U));
    BOOST_CHECK(Value(true) == Value(true));
    BOOST_CHECK(Value(1.1) == Value(1.1));
    BOOST_CHECK(Value() == nullValue);
    BOOST_CHECK(Value("abc") == Value("abc"));

    Value::ArrayType array1;
    array1.push_back(1);
    Value::ArrayType array2;
    array2.push_back(1);
    BOOST_CHECK(array1 == array2);

    Value::ObjectType object1;
    object1["test"] = 1;
    Value::ObjectType object2;
    object2["test"] = 1;
    BOOST_CHECK(object1 == object2);
}

BOOST_AUTO_TEST_CASE(SameTypeNotEqual_test)
{
    BOOST_CHECK(Value(1) != Value(2));
    BOOST_CHECK(Value(1U) != Value(2U));
    BOOST_CHECK(Value(true) != Value(false));
    BOOST_CHECK(Value(1.1) != Value(1.2));
    BOOST_CHECK(Value("abc") != Value("abd"));

    Value::ArrayType array1;
    array1.push_back(1);
    Value::ArrayType array2;
    array2.push_back(2);
    BOOST_CHECK(array1 != array2);

    Value::ObjectType object1;
    object1["test1"] = 1;
    Value::ObjectType object2;
    object2["test2"] = 1;
    BOOST_CHECK(object1 != object2);
}

BOOST_AUTO_TEST_CASE(DifferentTypeNotEqual_test)
{
    BOOST_CHECK(Value(1) != Value(1U));
    BOOST_CHECK(Value(true) != Value(1));
    BOOST_CHECK(Value(false) != nullValue);
    BOOST_CHECK(Value("1") != Value(1));

    Value::ArrayType array1;
    array1.push_back(1);

    Value::ObjectType object1;
    object1["test"] = 1;
    BOOST_CHECK(array1 != object1);
}

BOOST_AUTO_TEST_CASE(SameTypeLessThan_test)
{
    BOOST_CHECK(Value(1) < Value(2));
    BOOST_CHECK(Value(1U) < Value(2U));
    BOOST_CHECK(Value(false) < Value(true));
    BOOST_CHECK(Value(1.1) < Value(1.2));
    BOOST_CHECK(Value("abc") < Value("abd"));

    Value::ArrayType array1;
    array1.push_back(1);
    Value::ArrayType array2;
    array2.push_back(2);
    BOOST_CHECK(array1 < array2);

    Value::ObjectType object1;
    object1["test1"] = 1;
    Value::ObjectType object2;
    object2["test2"] = 1;
    BOOST_CHECK(object1 < object2);
}

BOOST_AUTO_TEST_CASE(DifferentTypeLessThan_test)
{
    BOOST_CHECK(lessXorGreater(Value(1), Value(1U)));
    BOOST_CHECK(lessXorGreater(Value(true), Value(1)));
    BOOST_CHECK(lessXorGreater(Value(false), nullValue));
    BOOST_CHECK(lessXorGreater(Value("1"), Value(1)));

    Value::ArrayType array1;
    array1.push_back(1);

    Value::ObjectType object1;
    object1["test"] = 1;
    BOOST_CHECK(lessXorGreater(array1, object1));
}

BOOST_AUTO_TEST_SUITE_END() // Compare_test
