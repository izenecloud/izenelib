#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>
#include <boost/shared_ptr.hpp>

#include <am/am_test/am_types.h>

#include <string>
#include <cstdio>

#define DIR_PREFIX "./tmp/am_IntString_"

using namespace izenelib::am;

typedef boost::mpl::list<
    tc::Hash<int, std::string>
    , tc::BTree<int, std::string>
    , tc::BTree<int, std::string, tc::BTreeLessCmp<int> >
    , sdb_hash<int, std::string>
    //, BTreeFile<int, std::string>
    //, cccr_hash<int, std::string>
    //, sdb_storage<int, std::string>
    //, SkipListFile<int, std::string>
    //, DynamicPerfectHash<int, std::string>
    //, LinearHashTable<int, std::string>
    //, Map<int, std::string>
    //, SkipList<std::string, int>
> test_types;

namespace { // {anonymous}

template<typename T>
boost::shared_ptr<T> create(const std::string& file)
{
    const std::string path = DIR_PREFIX + file;
    std::remove(path.c_str());
    boost::shared_ptr<T> result(new T(path));
    BOOST_REQUIRE(result);
    BOOST_REQUIRE(result->open());

    return result;
}
template<>
boost::shared_ptr<cccr_hash<int, std::string> > create(const std::string&)
{
    boost::shared_ptr<cccr_hash<int, std::string> >
        result(new cccr_hash<int, std::string>());

    BOOST_REQUIRE(result);

    return result;
}

} // namespace {anonymous}

BOOST_AUTO_TEST_SUITE(IntString_test)

BOOST_AUTO_TEST_CASE_TEMPLATE(NewIsEmpty_test, T, test_types)
{
    boost::shared_ptr<T> h = create<T>("NewIsEmpty_test");

    BOOST_CHECK(h->num_items() == 0);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(InsertGet_test, T, test_types)
{
    boost::shared_ptr<T> h = create<T>("InsertGet_test");

    BOOST_CHECK(h->insert(1, "value1"));

    std::string value;
    BOOST_CHECK(h->get(1, value));
    BOOST_CHECK(value == "value1");
}

BOOST_AUTO_TEST_CASE_TEMPLATE(GetFromEmtpy_test, T, test_types)
{
    boost::shared_ptr<T> h = create<T>("GetFromEmtpy_test");

    std::string value;
    BOOST_CHECK(!(h->get(1, value)));
    BOOST_CHECK(value.empty());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(GetNotExisted_test, T, test_types)
{
    boost::shared_ptr<T> h = create<T>("GetNotExisted_test");

    BOOST_CHECK(h->insert(1, "value"));

    std::string value;
    BOOST_CHECK(!h->get(2, value));
    BOOST_CHECK(value.empty());
}

BOOST_AUTO_TEST_CASE_TEMPLATE(InsertExisted_test, T, test_types)
{
    boost::shared_ptr<T> h = create<T>("InsertExisted_test");

    BOOST_CHECK(h->insert(1, "value1"));
    BOOST_CHECK(!h->insert(1, "value2"));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(InsertAndSize_test, T, test_types)
{
    boost::shared_ptr<T> h = create<T>("InsertAndSize_test");

    {
        BOOST_CHECK(h->insert(1, "value1"));
        BOOST_CHECK(h->num_items() == 1);
    }

    {
        BOOST_CHECK(h->insert(2, "value2"));
        BOOST_CHECK(h->num_items() == 2);
    }

    {
        BOOST_CHECK(h->insert(3, "value3"));
        BOOST_CHECK(h->num_items() == 3);
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(UpdateAsInsert_test, T, test_types)
{
    boost::shared_ptr<T> h = create<T>("UpdateAsInsert_test");

    BOOST_CHECK(h->update(1, "value1"));
    BOOST_CHECK(h->num_items() == 1);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Update_test, T, test_types)
{
    boost::shared_ptr<T> h = create<T>("Update_test");

    {
        BOOST_CHECK(h->insert(1, "value1"));
    }

    {
        BOOST_CHECK(h->update(1, "value2"));
        BOOST_CHECK(h->num_items() == 1);
    }

    {
        std::string value;
        BOOST_CHECK(h->get(1, value));
        BOOST_CHECK(value == "value2");
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Del_test, T, test_types)
{
    boost::shared_ptr<T> h = create<T>("Del_test");

    {
        BOOST_CHECK(h->insert(1, "value1"));
        BOOST_CHECK(h->num_items() == 1);
    }

    {
        BOOST_CHECK(h->del(1));
        BOOST_CHECK(h->num_items() == 0);
        std::string value;
        BOOST_CHECK(!h->get(1, value));
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(DelNotExisted_test, T, test_types)
{
    boost::shared_ptr<T> h = create<T>("DelNotExisted_test");

    {
        BOOST_CHECK(h->insert(1, "value1"));
        BOOST_CHECK(h->num_items() == 1);
    }

    {
        h->del(2);
        BOOST_CHECK(h->num_items() == 1);
        std::string value;
        BOOST_CHECK(h->get(1, value));
        BOOST_CHECK(value == "value1");
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Update1In2, T, test_types)
{
    boost::shared_ptr<T> h = create<T>("Update1In2_test");

    BOOST_CHECK(h->insert(1, "value1"));
    BOOST_CHECK(h->insert(2, "value2"));

    BOOST_CHECK(h->update(1, "value3"));

    BOOST_CHECK(h->num_items() == 2);
    std::string value;
    h->get(1, value);
    BOOST_CHECK(value == "value3");
    h->get(2, value);
    BOOST_CHECK(value == "value2");
}

BOOST_AUTO_TEST_CASE_TEMPLATE(Del1In2, T, test_types)
{
    boost::shared_ptr<T> h = create<T>("Del1In2_test");

    BOOST_CHECK(h->insert(1, "value1"));
    BOOST_CHECK(h->insert(2, "value2"));

    h->del(1);

    BOOST_CHECK(h->num_items() == 1);
    std::string value;
    BOOST_CHECK(!h->get(1, value));
    h->get(2, value);
    BOOST_CHECK(value == "value2");
}

BOOST_AUTO_TEST_SUITE_END()
