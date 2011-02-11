#include <boost/test/unit_test.hpp>

#include <util/driver/value/Value.h>
#include <util/driver/value/serialization.h>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <sstream>
#include <iostream>

using namespace izenelib::driver;

namespace { // {anonymous}
struct ValueFixture
{
    Value intValue;
    Value doubleValue;
    Value stringValue;
    Value arrayValue;
    Value objectValue;

    std::ostringstream* oss;
    std::istringstream* iss;
    boost::archive::binary_oarchive* oa;
    boost::archive::binary_iarchive* ia;

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

        oss = new std::ostringstream();
        oa = new boost::archive::binary_oarchive(*oss);
        ia = 0;
        iss = 0;
    }

    ~ValueFixture()
    {
        delete ia;
        delete oa;
        delete iss;
        delete oss;
        ia = 0;
        oa = 0;
        iss = 0;
        oss = 0;
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
        delete ia;
        delete iss;
        iss = new std::istringstream(oss->str());
        ia = new boost::archive::binary_iarchive(*iss);
    }
};

} // namespace {anonymous}

BOOST_FIXTURE_TEST_SUITE(BoostSerialization_test, ValueFixture)

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

BOOST_AUTO_TEST_SUITE_END() // BoostSerialization_test
