#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

#include <string>
#include <time.h>
#include <math.h>
#include <fstream>
#include <iostream>

#include <cstdio>

#include <sdb/SequentialDB.h>

#define DIR_PREFIX "./tmp/am_tc_Db_"

using namespace izenelib::am;
using namespace izenelib::sdb;
using namespace std;
namespace bfs = boost::filesystem;

using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE(tc_HashDb_test)

BOOST_AUTO_TEST_CASE(tc_hash)
{
    bfs::path db_dir(DIR_PREFIX);
    boost::filesystem::remove_all(db_dir);
    bfs::create_directories(db_dir);
    std::string db_dir_str = db_dir.string();
    typedef std::vector<std::pair<int, std::string> > ValueType;
    typedef SequentialDB<std::string, ValueType> TcHashType;

    std::cout << "first insert testing" << std::endl;
    TcHashType h(db_dir_str+"/tchash_test");
    BOOST_CHECK(h.open());

    for (int i = 0; i < 100000; ++i)
    {
        {
            std::string key("1查询中文");
            ValueType value;
            value.push_back(std::make_pair(1, "a"));

            BOOST_CHECK(h.update(key, value));
        }

        {
            std::string key("2dddidddfdfi12fldjf");
            ValueType value;
            value.push_back(std::make_pair(1, "a"));
            value.push_back(std::make_pair(2, "b"));

            BOOST_CHECK(h.update(key, value));
        }

        {
            std::string key("3狗狗哦手机iphone哦");
            ValueType value;
            value.push_back(std::make_pair(1, "a"));
            value.push_back(std::make_pair(2, "b"));
            value.push_back(std::make_pair(3, "c"));

            BOOST_CHECK(h.update(key, value));
        }
    }
    h.flush();

    std::cout << "first finished insert, wait check" << std::endl;
    sleep(1);
    std::cout << "reopen first" << std::endl;
    h.close();
    h.open();
    h.flush();
    sleep(1);
    std::cout << "reopen first 2" << std::endl;
    h.close();
    h.open();
    h.flush();
    sleep(1);
    std::cout << "reopen first 3" << std::endl;
    h.close();
    h.open();
    h.flush();
    sleep(1);

    std::cout << "second insert testing" << std::endl;
    TcHashType h2(db_dir_str+"/tchash_test_2");
    BOOST_CHECK(h2.open());
    for (int i = 0; i < 100000; ++i)
    {
        {
            std::string key("1查询中文");
            ValueType value;
            value.push_back(std::make_pair(1, "a"));

            BOOST_CHECK(h2.update(key, value));
        }

        {
            std::string key("2dddidddfdfi12fldjf");
            ValueType value;
            value.push_back(std::make_pair(1, "a"));
            value.push_back(std::make_pair(2, "b"));

            BOOST_CHECK(h2.update(key, value));
        }

        {
            std::string key("3狗狗哦手机iphone哦");
            ValueType value;
            value.push_back(std::make_pair(1, "a"));
            value.push_back(std::make_pair(2, "b"));
            value.push_back(std::make_pair(3, "c"));

            BOOST_CHECK(h2.update(key, value));
        }
    }
    h2.flush();

    std::cout << "second finished insert, wait check" << std::endl;
    sleep(1);
    std::cout << "reopen second" << std::endl;
    h2.close();
    h2.open();
    h2.flush();
    sleep(1);
    std::cout << "reopen second 2" << std::endl;
    h2.close();
    h2.open();
    h2.flush();
    sleep(1);
    std::cout << "reopen second 3" << std::endl;
    h2.close();
    h2.open();
    h2.flush();
    sleep(1);
}

BOOST_AUTO_TEST_SUITE_END() // tc_Db_test
