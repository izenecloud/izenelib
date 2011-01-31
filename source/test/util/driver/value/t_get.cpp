#include <boost/test/unit_test.hpp>

#include <util/driver/value/Value.h>

using namespace izenelib::driver;

namespace { // {anonymous}
struct ValueFixture
{
    Value intValue;
    Value uintValue;
    Value boolValue;
    Value doubleValue;
    Value stringValue;
    Value arrayValue;
    Value objectValue;

    ValueFixture()
    : intValue(1)
    , uintValue(10U)
    , boolValue(true)
    , doubleValue(2.0)
    , stringValue("test")
    {
        Value::ArrayType array;
        array.push_back(1);
        arrayValue = array;

        Value::ObjectType object;
        object["test"] = 1;
        objectValue = object;
    }

    const Value& asConst(const Value& v)
    {
        return v;
    }

    const Value* asConst(const Value* v)
    {
        return v;
    }
};

} // namespace {anonymous}


BOOST_FIXTURE_TEST_SUITE(Get_test, ValueFixture)

BOOST_AUTO_TEST_CASE(GetPtr_Mutable_NameInFunc_test)
{
    BOOST_CHECK(intValue.getNullPtr() == intValue.getPtr<Value::NullType>());
    BOOST_CHECK(intValue.getIntPtr() == intValue.getPtr<Value::IntType>());
    BOOST_CHECK(intValue.getUintPtr() == intValue.getPtr<Value::UintType>());
    BOOST_CHECK(intValue.getDoublePtr() == intValue.getPtr<Value::DoubleType>());
    BOOST_CHECK(intValue.getStringPtr() == intValue.getPtr<Value::StringType>());
    BOOST_CHECK(intValue.getBoolPtr() == intValue.getPtr<Value::BoolType>());
    BOOST_CHECK(intValue.getArrayPtr() == intValue.getPtr<Value::ArrayType>());
    BOOST_CHECK(intValue.getObjectPtr() == intValue.getPtr<Value::ObjectType>());
}

BOOST_AUTO_TEST_CASE(GetPtr_Mutable_MatchType_test)
{
    Value::IntType* intPtr = intValue.getPtr<Value::IntType>();
    BOOST_CHECK(intPtr);
    BOOST_CHECK(*intPtr == 1);

    *intPtr = 10;
    BOOST_CHECK(intValue == 10);
}

BOOST_AUTO_TEST_CASE(GetPtr_Mutable_UnmatchType_test)
{
    Value::IntType* intPtr = doubleValue.getPtr<Value::IntType>();
    BOOST_CHECK(!intPtr);
}

BOOST_AUTO_TEST_CASE(GetPtr_Const_NameInFunc_test)
{
    BOOST_CHECK(asConst(intValue).getNullPtr() == asConst(intValue).getPtr<Value::NullType>());
    BOOST_CHECK(asConst(intValue).getIntPtr() == asConst(intValue).getPtr<Value::IntType>());
    BOOST_CHECK(asConst(intValue).getUintPtr() == asConst(intValue).getPtr<Value::UintType>());
    BOOST_CHECK(asConst(intValue).getDoublePtr() == asConst(intValue).getPtr<Value::DoubleType>());
    BOOST_CHECK(asConst(intValue).getStringPtr() == asConst(intValue).getPtr<Value::StringType>());
    BOOST_CHECK(asConst(intValue).getBoolPtr() == asConst(intValue).getPtr<Value::BoolType>());
    BOOST_CHECK(asConst(intValue).getArrayPtr() == asConst(intValue).getPtr<Value::ArrayType>());
    BOOST_CHECK(asConst(intValue).getObjectPtr() == asConst(intValue).getPtr<Value::ObjectType>());
}

BOOST_AUTO_TEST_CASE(GetPtr_Const_MatchType_test)
{
    const Value::IntType* intPtr = asConst(intValue).getPtr<Value::IntType>();
    BOOST_CHECK(intPtr);
    BOOST_CHECK(*intPtr == 1);
}

BOOST_AUTO_TEST_CASE(GetPtr_Const_UnmatchType_test)
{
    const Value::IntType* intPtr = asConst(doubleValue).getPtr<Value::IntType>();
    BOOST_CHECK(!intPtr);
}

BOOST_AUTO_TEST_CASE(GetPtrOrReset_Mutable_NameInFunc_test)
{
    BOOST_CHECK(intValue.getNullPtrOrReset() == intValue.getPtrOrReset<Value::NullType>());
    BOOST_CHECK(intValue.getIntPtrOrReset() == intValue.getPtrOrReset<Value::IntType>());
    BOOST_CHECK(intValue.getUintPtrOrReset() == intValue.getPtrOrReset<Value::UintType>());
    BOOST_CHECK(intValue.getDoublePtrOrReset() == intValue.getPtrOrReset<Value::DoubleType>());
    BOOST_CHECK(intValue.getStringPtrOrReset() == intValue.getPtrOrReset<Value::StringType>());
    BOOST_CHECK(intValue.getBoolPtrOrReset() == intValue.getPtrOrReset<Value::BoolType>());
    BOOST_CHECK(intValue.getArrayPtrOrReset() == intValue.getPtrOrReset<Value::ArrayType>());
    BOOST_CHECK(intValue.getObjectPtrOrReset() == intValue.getPtrOrReset<Value::ObjectType>());
}

BOOST_AUTO_TEST_CASE(GetPtrOrReset_Mutable_MatchType_test)
{
    Value::IntType* intPtr = intValue.getPtrOrReset<Value::IntType>();
    BOOST_CHECK(intPtr);
    BOOST_CHECK(*intPtr == 1);

    *intPtr = 10;
    BOOST_CHECK(intValue == 10);
}

BOOST_AUTO_TEST_CASE(GetPtrOrReset_Mutable_UnmatchType_test)
{
    Value::IntType* intPtr = doubleValue.getPtrOrReset<Value::IntType>();
    BOOST_CHECK(intPtr);
    BOOST_CHECK(doubleValue.type() == Value::kIntType);
    BOOST_CHECK(*intPtr == 0);

    *intPtr = 10;
    BOOST_CHECK(doubleValue == 10);
}

BOOST_AUTO_TEST_CASE(GetRef_Mutable_NameInFunc_test)
{
    Value null;
    BOOST_CHECK(null.getNull() == null.get<Value::NullType>());
    BOOST_CHECK(intValue.getInt() == intValue.get<Value::IntType>());
    BOOST_CHECK(uintValue.getUint() == uintValue.get<Value::UintType>());
    BOOST_CHECK(doubleValue.getDouble() == doubleValue.get<Value::DoubleType>());
    BOOST_CHECK(stringValue.getString() == stringValue.get<Value::StringType>());
    BOOST_CHECK(boolValue.getBool() == boolValue.get<Value::BoolType>());
    BOOST_CHECK(arrayValue.getArray() == arrayValue.get<Value::ArrayType>());
    BOOST_CHECK(objectValue.getObject() == objectValue.get<Value::ObjectType>());
}

BOOST_AUTO_TEST_CASE(GetRef_Mutable_MatchType_test)
{
    Value::IntType& intPtr = intValue.get<Value::IntType>();
    BOOST_CHECK(intPtr == 1);

    intPtr = 10;
    BOOST_CHECK(intValue == 10);
}

BOOST_AUTO_TEST_CASE(GetRef_Mutable_UnmatchType_test)
{
    BOOST_CHECK_THROW(doubleValue.get<Value::IntType>(), bad_get);
}

BOOST_AUTO_TEST_CASE(GetRefOrReset_Mutable_NameInFunc_test)
{
    Value null;
    BOOST_CHECK(null.getNullOrReset() == null.getOrReset<Value::NullType>());
    BOOST_CHECK(intValue.getIntOrReset() == intValue.getOrReset<Value::IntType>());
    BOOST_CHECK(uintValue.getUintOrReset() == uintValue.getOrReset<Value::UintType>());
    BOOST_CHECK(doubleValue.getDoubleOrReset() == doubleValue.getOrReset<Value::DoubleType>());
    BOOST_CHECK(stringValue.getStringOrReset() == stringValue.getOrReset<Value::StringType>());
    BOOST_CHECK(boolValue.getBoolOrReset() == boolValue.getOrReset<Value::BoolType>());
    BOOST_CHECK(arrayValue.getArrayOrReset() == arrayValue.getOrReset<Value::ArrayType>());
    BOOST_CHECK(objectValue.getObjectOrReset() == objectValue.getOrReset<Value::ObjectType>());
}

BOOST_AUTO_TEST_CASE(GetRefOrReset_Mutable_MatchType_test)
{
    Value::IntType& intRef = intValue.getOrReset<Value::IntType>();
    BOOST_CHECK(intRef == 1);

    intRef = 10;
    BOOST_CHECK(intValue == 10);
}

BOOST_AUTO_TEST_CASE(GetRefOrReset_Mutable_UnmatchType_test)
{
    Value::IntType& intRef = doubleValue.getOrReset<Value::IntType>();
    BOOST_CHECK(intRef == 0);
    BOOST_CHECK(doubleValue.type() == Value::kIntType);

    intRef = 10;
    BOOST_CHECK(doubleValue == 10);
}

BOOST_AUTO_TEST_CASE(GetRef_Const_NameInFunc_test)
{
    const Value null;
    BOOST_CHECK(null.getNull() == null.get<Value::NullType>());
    BOOST_CHECK(asConst(intValue).getInt() == asConst(intValue).get<Value::IntType>());
    BOOST_CHECK(asConst(uintValue).getUint() == asConst(uintValue).get<Value::UintType>());
    BOOST_CHECK(asConst(doubleValue).getDouble() == asConst(doubleValue).get<Value::DoubleType>());
    BOOST_CHECK(asConst(stringValue).getString() == asConst(stringValue).get<Value::StringType>());
    BOOST_CHECK(asConst(boolValue).getBool() == asConst(boolValue).get<Value::BoolType>());
    BOOST_CHECK(asConst(arrayValue).getArray() == asConst(arrayValue).get<Value::ArrayType>());
    BOOST_CHECK(asConst(objectValue).getObject() == asConst(objectValue).get<Value::ObjectType>());
}

BOOST_AUTO_TEST_CASE(GetRef_Const_MatchType_test)
{
    const Value::IntType& intPtr = asConst(intValue).get<Value::IntType>();
    BOOST_CHECK(intPtr);
    BOOST_CHECK(intPtr == 1);
}

BOOST_AUTO_TEST_CASE(GetRef_Const_UnmatchType_test)
{
    BOOST_CHECK_THROW(asConst(doubleValue).get<Value::IntType>(), bad_get);
}



BOOST_AUTO_TEST_CASE(GetPtr_Mutable_Free_MatchType_test)
{
    Value::IntType* intPtr = get<Value::IntType>(&intValue);
    BOOST_CHECK(intPtr);
    BOOST_CHECK(*intPtr == 1);
}

BOOST_AUTO_TEST_CASE(GetPtr_Mutable_Free_UnmatchType_test)
{
    Value::IntType* intPtr = get<Value::IntType>(&doubleValue);
    BOOST_CHECK(!intPtr);
}

BOOST_AUTO_TEST_CASE(GetPtrOrReset_Free_MatchType_test)
{
    Value::IntType* intPtr = getOrReset<Value::IntType>(&intValue);
    BOOST_CHECK(intPtr);
    BOOST_CHECK(*intPtr == 1);
}

BOOST_AUTO_TEST_CASE(GetPtrOrReset_Mutable_Free_UnmatchType_test)
{
    Value::IntType* intPtr = getOrReset<Value::IntType>(&doubleValue);
    BOOST_CHECK(intPtr);
    BOOST_CHECK(*intPtr == 0);
}

BOOST_AUTO_TEST_CASE(GetPtr_Const_Free_MatchType_test)
{
    const Value::IntType* intPtr = get<Value::IntType>(asConst(&intValue));
    BOOST_CHECK(intPtr);
    BOOST_CHECK(*intPtr == 1);
}

BOOST_AUTO_TEST_CASE(GetPtr_Const_Free_UnmatchType_test)
{
    const Value::IntType* intPtr = get<Value::IntType>(asConst(&doubleValue));
    BOOST_CHECK(!intPtr);
}

BOOST_AUTO_TEST_CASE(GetRef_Mutable_Free_MatchType_test)
{
    Value::IntType& intPtr = get<Value::IntType>(intValue);
    BOOST_CHECK(intPtr == 1);
}

BOOST_AUTO_TEST_CASE(GetRef_Mutable_Free_UnmatchType_test)
{
    BOOST_CHECK_THROW(get<Value::IntType>(doubleValue), bad_get);
}

BOOST_AUTO_TEST_CASE(GetRefOrReset_Free_MatchType_test)
{
    Value::IntType& intPtr = getOrReset<Value::IntType>(intValue);
    BOOST_CHECK(intPtr == 1);
}

BOOST_AUTO_TEST_CASE(GetRefOrReset_Free_UnmatchType_test)
{
    Value::IntType& intPtr = getOrReset<Value::IntType>(doubleValue);
    BOOST_CHECK(intPtr == 0);
}

BOOST_AUTO_TEST_CASE(GetRef_Const_Free_MatchType_test)
{
    const Value::IntType& intPtr = get<Value::IntType>(asConst(intValue));
    BOOST_CHECK(intPtr);
    BOOST_CHECK(intPtr == 1);
}

BOOST_AUTO_TEST_CASE(GetRef_Const_Free_UnmatchType_test)
{
    BOOST_CHECK_THROW(get<Value::IntType>(asConst(doubleValue)), bad_get);
}

BOOST_AUTO_TEST_SUITE_END() // Get_test
