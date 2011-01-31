#include <boost/test/unit_test.hpp>

#include <util/driver/value/Value.h>
#include <util/driver/value/serialization.h>

#include <3rdparty/febird/io/DataIO.h>
#include <3rdparty/febird/io/MemStream.h>

#include <sstream>
#include <iostream>

using namespace izenelib::driver;
using namespace febird;

namespace { // {anonymous}
struct ValueFixture
{
    Value intValue;
    Value doubleValue;
    Value stringValue;
    Value arrayValue;
    Value objectValue;

    PortableDataOutput<AutoGrownMemIO>* oa;
    PortableDataInput<MemIO>* ia;

    ValueFixture()
    : intValue(1)
    , doubleValue(2.0)
    , stringValue("test")
    {
        Value::ArrayType array;
        array.push_back(1);
        arrayValue = array;

        Value::ObjectType object;
        object["test"] = 1;
        objectValue = object;

        oa = new PortableDataOutput<AutoGrownMemIO>();
        ia = new PortableDataInput<MemIO>();
    }

    ~ValueFixture()
    {
        delete ia;
        delete oa;
        ia = 0;
        oa = 0;
    }

    const Value& asConst(const Value& v)
    {
        return v;
    }

    const Value* asConst(const Value* v)
    {
        return v;
    }

    void transfer()
    {
        ia->set(oa->begin(), oa->current());
    }
};

} // namespace {anonymous}

BOOST_FIXTURE_TEST_SUITE(FebirdSerialization_test, ValueFixture)

BOOST_AUTO_TEST_CASE(Default_test)
{
    Value src;
    (*oa) << src;

    transfer();

    Value dest;
    (*ia) >> dest;

    BOOST_CHECK(src == dest);
}

BOOST_AUTO_TEST_CASE(NullValue_test)
{
    Value src(nullValue);
    (*oa) << src;

    transfer();

    Value dest;
    (*ia) >> dest;

    BOOST_CHECK(src == dest);
}

BOOST_AUTO_TEST_CASE(IntValue_test)
{
    Value src(10);
    (*oa) << src;

    transfer();

    Value dest;
    (*ia) >> dest;

    BOOST_CHECK(src == dest);
}

BOOST_AUTO_TEST_CASE(UintValue_test)
{
    Value src(10U);
    (*oa) << src;

    transfer();

    Value dest;
    (*ia) >> dest;

    BOOST_CHECK(src == dest);
}

BOOST_AUTO_TEST_CASE(DoubleValue_test)
{
    Value src(1.5);
    (*oa) << src;

    transfer();

    Value dest;
    (*ia) >> dest;

    BOOST_CHECK(src == dest);
}

BOOST_AUTO_TEST_CASE(StringValue_test)
{
    Value src("test");
    (*oa) << src;

    transfer();

    Value dest;
    (*ia) >> dest;

    BOOST_CHECK(src == dest);
}

BOOST_AUTO_TEST_CASE(VectorValue_test)
{
    Value::ArrayType vec;
    vec.push_back("test");
    vec.push_back("10");

    Value src(vec);

    (*oa) << src;

    transfer();

    Value dest;
    (*ia) >> dest;

    BOOST_CHECK(src == dest);
}

BOOST_AUTO_TEST_CASE(ObjectValue_test)
{
    Value::ObjectType obj;
    obj["test"] = 1.5;
    obj["tese"] = false;

    Value src(obj);

    (*oa) << src;

    transfer();

    Value dest;
    (*ia) >> dest;

    BOOST_CHECK(src == dest);
}

BOOST_AUTO_TEST_SUITE_END() // FebirdSerialization_test
