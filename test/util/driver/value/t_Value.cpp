#include <boost/test/unit_test.hpp>

#include <util/driver/value/Value.h>

using namespace izenelib::driver;

BOOST_AUTO_TEST_SUITE(Value_test)

BOOST_AUTO_TEST_CASE(DefaultCtor_test)
{
    Value value;
    BOOST_CHECK(nullValue(value));
    BOOST_CHECK(value.type() == Value::kNullType);
}

BOOST_AUTO_TEST_CASE(NullCtor_test)
{
    Value value(nullValue);
    BOOST_CHECK(nullValue(value));
    BOOST_CHECK(value.type() == Value::kNullType);
}

// It may cause ambiguous
BOOST_AUTO_TEST_CASE(ZeroCtor_test)
{
    Value value(0);
    BOOST_CHECK(!nullValue(value));
    BOOST_CHECK(value.type() == Value::kIntType);
    BOOST_CHECK(value.getInt() == 0);

}

BOOST_AUTO_TEST_CASE(IntCtor_test)
{
    {
        Value value(-1);
        BOOST_CHECK(!nullValue(value));
        BOOST_CHECK(value.type() == Value::kIntType);
        BOOST_CHECK(value.getInt() == -1);
    }

    {
        Value value(1);
        BOOST_CHECK(!nullValue(value));
        BOOST_CHECK(value.type() == Value::kIntType);
        BOOST_CHECK(value.getInt() == 1);
    }
}

BOOST_AUTO_TEST_CASE(LongCtor_test)
{
    {
        long i = -1;
        Value value(i);
        BOOST_CHECK(!nullValue(value));
        BOOST_CHECK(value.type() == Value::kIntType);
        BOOST_CHECK(value.getInt() == i);
    }

    {
        long i = 1;
        Value value(i);
        BOOST_CHECK(!nullValue(value));
        BOOST_CHECK(value.type() == Value::kIntType);
        BOOST_CHECK(value.getInt() == i);
    }
}

BOOST_AUTO_TEST_CASE(UnsignedCtor_test)
{
    Value value(0U);
    BOOST_CHECK(!nullValue(value));
    BOOST_CHECK(value.type() == Value::kUintType);
    BOOST_CHECK(value.getUint() == 0);
}

BOOST_AUTO_TEST_CASE(SizeTCtor_test)
{
    std::size_t size(100);
    Value value(size);
    BOOST_CHECK(!nullValue(value));
    BOOST_CHECK(value.type() == Value::kUintType);
    BOOST_CHECK(value.getUint() == size);
}

BOOST_AUTO_TEST_CASE(TrueCtor_test)
{
    Value value(true);
    BOOST_CHECK(!nullValue(value));
    BOOST_CHECK(value.type() == Value::kBoolType);
    BOOST_CHECK(value.getBool() == true);
}

BOOST_AUTO_TEST_CASE(FalseCtor_test)
{
    Value value(false);
    BOOST_CHECK(!nullValue(value));
    BOOST_CHECK(value.type() == Value::kBoolType);
    BOOST_CHECK(value.getBool() == false);
}

BOOST_AUTO_TEST_CASE(CstrCtor_test)
{
    Value value("test");
    BOOST_CHECK(!nullValue(value));
    BOOST_CHECK(value.type() == Value::kStringType);
    BOOST_CHECK(value.getString() == "test");
}

BOOST_AUTO_TEST_CASE(StrCtor_test)
{
    std::string str("test");
    Value value(str);
    BOOST_CHECK(!nullValue(value));
    BOOST_CHECK(value.type() == Value::kStringType);
    BOOST_CHECK(value.getString() == str);
}

BOOST_AUTO_TEST_CASE(DoubleCtor_test)
{
    Value value(0.0);
    BOOST_CHECK(!nullValue(value));
    BOOST_CHECK(value.type() == Value::kDoubleType);
    BOOST_CHECK(value.getDouble() == 0.0);
}

BOOST_AUTO_TEST_CASE(FloatCtor_test)
{
    Value value(0.0F);
    BOOST_CHECK(!nullValue(value));
    BOOST_CHECK(value.type() == Value::kDoubleType);
    BOOST_CHECK(value.getDouble() == 0.0);
}

BOOST_AUTO_TEST_CASE(ArrayCtor_test)
{
    Value::ArrayType array;
    array.push_back(1);
    array.push_back(1.1);

    Value value(array);
    BOOST_CHECK(!nullValue(value));
    BOOST_CHECK(value.type() == Value::kArrayType);
    BOOST_CHECK(value.getArray() == array);
}

BOOST_AUTO_TEST_CASE(ObjectCtor_test)
{
    Value::ObjectType object;
    object["a"] = 1;
    object["b"] = 1.1;

    Value value(object);
    BOOST_CHECK(!nullValue(value));
    BOOST_CHECK(value.type() == Value::kObjectType);
    BOOST_CHECK(value.getObject() == object);
}

BOOST_AUTO_TEST_CASE(CopyCtor_test)
{
    Value value(10);
    Value copied(value);

    BOOST_CHECK(copied.type() == Value::kIntType);
    BOOST_CHECK(copied.getInt() == 10);
}

BOOST_AUTO_TEST_CASE(NullAssignment_test)
{
    Value value(10);
    value = nullValue;
    BOOST_CHECK(nullValue(value));
    BOOST_CHECK(value.type() == Value::kNullType);
}

BOOST_AUTO_TEST_CASE(ZeroAssignment_test)
{
    Value value;
    value = 0;
    BOOST_CHECK(!nullValue(value));
    BOOST_CHECK(value.type() == Value::kIntType);
    BOOST_CHECK(value.getInt() == 0);

}

BOOST_AUTO_TEST_CASE(IntAssignment_test)
{
    {
        Value value;
        value = -1;
        BOOST_CHECK(!nullValue(value));
        BOOST_CHECK(value.type() == Value::kIntType);
        BOOST_CHECK(value.getInt() == -1);
    }

    {
        Value value;
        value = 1;
        BOOST_CHECK(!nullValue(value));
        BOOST_CHECK(value.type() == Value::kIntType);
        BOOST_CHECK(value.getInt() == 1);
    }
}

BOOST_AUTO_TEST_CASE(LongAssignment_test)
{
    {
        long i = -1;
        Value value;
        value = i;
        BOOST_CHECK(!nullValue(value));
        BOOST_CHECK(value.type() == Value::kIntType);
        BOOST_CHECK(value.getInt() == i);
    }

    {
        long i = 1;
        Value value;
        value = i;
        BOOST_CHECK(!nullValue(value));
        BOOST_CHECK(value.type() == Value::kIntType);
        BOOST_CHECK(value.getInt() == i);
    }
}

BOOST_AUTO_TEST_CASE(UnsignedAssignment_test)
{
    Value value;
    value = 0U;
    BOOST_CHECK(!nullValue(value));
    BOOST_CHECK(value.type() == Value::kUintType);
    BOOST_CHECK(value.getUint() == 0);
}

BOOST_AUTO_TEST_CASE(SizeTAssignemnt_test)
{
    std::size_t size(100);
    Value value;
    value = size;
    BOOST_CHECK(!nullValue(value));
    BOOST_CHECK(value.type() == Value::kUintType);
    BOOST_CHECK(value.getUint() == size);
}

BOOST_AUTO_TEST_CASE(TrueAssignment_test)
{
    Value value;
    value = true;
    BOOST_CHECK(!nullValue(value));
    BOOST_CHECK(value.type() == Value::kBoolType);
    BOOST_CHECK(value.getBool() == true);
}

BOOST_AUTO_TEST_CASE(FalseAssignment_test)
{
    Value value;
    value = false;
    BOOST_CHECK(!nullValue(value));
    BOOST_CHECK(value.type() == Value::kBoolType);
    BOOST_CHECK(value.getBool() == false);
}

BOOST_AUTO_TEST_CASE(CstrAssignemnt_test)
{
    Value value;
    value = "test";
    BOOST_CHECK(!nullValue(value));
    BOOST_CHECK(value.type() == Value::kStringType);
    BOOST_CHECK(value.getString() == "test");
}

BOOST_AUTO_TEST_CASE(StrAssignment_test)
{
    std::string str("test");
    Value value;
    value = str;
    BOOST_CHECK(!nullValue(value));
    BOOST_CHECK(value.type() == Value::kStringType);
    BOOST_CHECK(value.getString() == str);
}

BOOST_AUTO_TEST_CASE(ArrayAssignemnt_test)
{
    Value::ArrayType array;
    array.push_back(1);
    array.push_back(1.1);

    Value value;
    value = array;
    BOOST_CHECK(!nullValue(value));
    BOOST_CHECK(value.type() == Value::kArrayType);
    BOOST_CHECK(value.getArray() == array);
}

BOOST_AUTO_TEST_CASE(ObjectAssignment_test)
{
    Value::ObjectType object;
    object["a"] = 1;
    object["b"] = 1.1;

    Value value;
    value = object;
    BOOST_CHECK(!nullValue(value));
    BOOST_CHECK(value.type() == Value::kObjectType);
    BOOST_CHECK(value.getObject() == object);
}

BOOST_AUTO_TEST_CASE(Swap_test)
{
    Value a(10);
    Value b("test");

    swap(a, b);
    BOOST_CHECK(a.type() == Value::kStringType);
    BOOST_CHECK(b.type() == Value::kIntType);

    BOOST_CHECK(a.getString() == "test");
    BOOST_CHECK(b.getInt() == 10);
}

BOOST_AUTO_TEST_CASE(DoubleAssignment_test)
{
    Value value;
    value = 0.0;
    BOOST_CHECK(!nullValue(value));
    BOOST_CHECK(value.type() == Value::kDoubleType);
    BOOST_CHECK(value.getDouble() == 0.0);
}

BOOST_AUTO_TEST_CASE(FloatAssignment_test)
{
    Value value;
    value = 0.0F;
    BOOST_CHECK(!nullValue(value));
    BOOST_CHECK(value.type() == Value::kDoubleType);
    BOOST_CHECK(value.getDouble() == 0.0);
}

BOOST_AUTO_TEST_SUITE_END() // Value_test
