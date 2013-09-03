#include "InvertedIndexTestFixture.h"
#include <boost/test/unit_test.hpp>
NS_IZENELIB_IR_BEGIN
using namespace Zambezi;

BOOST_AUTO_TEST_SUITE(t_index_search)

BOOST_AUTO_TEST_CASE(do_search_BWAND_AND_revserse)
{
    uint32_t DocNum = 300000;
    InvertedIndexTestFixture indexTestFixture;
    bool reverse = true;
    indexTestFixture.initIndexer(DocNum, reverse);

    std::vector<std::string> term_list;
    std::vector<uint32_t> docid_list;
    term_list.push_back("abc");
    term_list.push_back("abd");
    term_list.push_back("abe");
    indexTestFixture.search(term_list, docid_list, BWAND_AND);
    uint32_t count = 20;
    BOOST_CHECK_EQUAL(docid_list.size(), 260000);

    for (std::vector<uint32_t>::iterator it = docid_list.begin(); it != docid_list.end(); ++it)
    {
        count--;
        if (19 == count)
           BOOST_CHECK_EQUAL(*it, 300000);
        if (18 == count)
           BOOST_CHECK_EQUAL(*it, 299999);
        if (17 == count)
           BOOST_CHECK_EQUAL(*it, 299998);
        if (16 == count)
           BOOST_CHECK_EQUAL(*it, 299997);
        if (15 == count)
           BOOST_CHECK_EQUAL(*it, 299996); 
        if (14 == count)
           BOOST_CHECK_EQUAL(*it, 299995);
        if (13 == count)
           BOOST_CHECK_EQUAL(*it, 299994);
        if (12 == count)
           BOOST_CHECK_EQUAL(*it, 299993);
        if (11 == count)
           BOOST_CHECK_EQUAL(*it, 299992);
        if (10 == count)
           BOOST_CHECK_EQUAL(*it, 299991);
        if (9 == count)
           BOOST_CHECK_EQUAL(*it, 299990);
        if (8 == count)
           BOOST_CHECK_EQUAL(*it, 299989);
        if (7 == count)
           BOOST_CHECK_EQUAL(*it, 299988);
        if (6 == count)
           BOOST_CHECK_EQUAL(*it, 299985); // ok
        if (5 == count)
           BOOST_CHECK_EQUAL(*it, 299984); // ok
        if (4 == count)
           BOOST_CHECK_EQUAL(*it, 299983); // ok
    }

    term_list.clear();
    docid_list.clear();
    term_list.push_back("abd");
    term_list.push_back("abq");
    indexTestFixture.search(term_list, docid_list, BWAND_AND);
    count = 20;
    BOOST_CHECK_EQUAL(docid_list.size(), 20000);
    for (std::vector<uint32_t>::iterator it = docid_list.begin(); it != docid_list.end(); ++it)
    {
        count--;
        if (19 == count)
           BOOST_CHECK_EQUAL(*it, 300000);
        if (18 == count)
           BOOST_CHECK_EQUAL(*it, 299985);
        if (17 == count)
           BOOST_CHECK_EQUAL(*it, 299970);
        if (16 == count)
        {
           BOOST_CHECK_EQUAL(*it, 299955);
           break;
        }
    }
}

BOOST_AUTO_TEST_CASE(do_search_BWAND_AND_forward)
{
    uint32_t DocNum = 300000;
    InvertedIndexTestFixture indexTestFixture;
    bool reverse = false;
    indexTestFixture.initIndexer(DocNum, reverse);

    std::vector<std::string> term_list;
    std::vector<uint32_t> docid_list;
    term_list.push_back("abc");
    term_list.push_back("abd");
    term_list.push_back("abe");
    indexTestFixture.search(term_list, docid_list, BWAND_AND);
    uint32_t count = 20;
    BOOST_CHECK_EQUAL(docid_list.size(), 260000);

    for (std::vector<uint32_t>::iterator it = docid_list.begin(); it != docid_list.end(); ++it)
    {
        count--;
        if (19 == count)
           BOOST_CHECK_EQUAL(*it, 3);
        if (18 == count)
           BOOST_CHECK_EQUAL(*it, 4);
        if (17 == count)
           BOOST_CHECK_EQUAL(*it, 5);
        if (16 == count)
           BOOST_CHECK_EQUAL(*it, 6);
        if (15 == count)
           BOOST_CHECK_EQUAL(*it, 7); 
        if (14 == count)
           BOOST_CHECK_EQUAL(*it, 8);
        if (13 == count)
           BOOST_CHECK_EQUAL(*it, 9);
        if (12 == count)
           BOOST_CHECK_EQUAL(*it, 10);
        if (11 == count)
           BOOST_CHECK_EQUAL(*it, 11);
        if (10 == count)
           BOOST_CHECK_EQUAL(*it, 12);
        if (9 == count)
           BOOST_CHECK_EQUAL(*it, 13);
        if (8 == count)
           BOOST_CHECK_EQUAL(*it, 14);
        if (7 == count)
           BOOST_CHECK_EQUAL(*it, 15);
        if (6 == count)
           BOOST_CHECK_EQUAL(*it, 18); // ok
        if (5 == count)
           BOOST_CHECK_EQUAL(*it, 19); // ok
        if (4 == count)
           BOOST_CHECK_EQUAL(*it, 20); // ok
    }

    term_list.clear();
    docid_list.clear();
    term_list.push_back("abd");
    term_list.push_back("abq");
    indexTestFixture.search(term_list, docid_list, BWAND_AND);
    count = 20;
    BOOST_CHECK_EQUAL(docid_list.size(), 20000);
    for (std::vector<uint32_t>::iterator it = docid_list.begin(); it != docid_list.end(); ++it)
    {
        count--;
        if (19 == count)
           BOOST_CHECK_EQUAL(*it, 15);
        if (18 == count)
           BOOST_CHECK_EQUAL(*it, 30);
        if (17 == count)
           BOOST_CHECK_EQUAL(*it, 45);
        if (16 == count)
        {
           BOOST_CHECK_EQUAL(*it, 60);
           break;
        }
    }
}
/*
BOOST_AUTO_TEST_CASE(do_search_BWAND_OR)
{
    InvertedIndexTestFixture indexTestFixture;
    indexTestFixture.initIndexer(index);
    std::cout << "term number:" << index->DictionarySize() << std::endl;

    std::vector<std::string> term_list;
    std::vector<uint32_t> docid_list;
    term_list.push_back("abc");
    indexTestFixture.search(term_list, docid_list, BWAND_OR);
    BOOST_CHECK_EQUAL(docid_list.size(), 15);

    term_list.clear();
    docid_list.clear();
    term_list.push_back("abd");
    indexTestFixture.search(term_list, docid_list, BWAND_OR);
    BOOST_CHECK_EQUAL(docid_list.size(), 14);

    term_list.clear();
    docid_list.clear();
    term_list.push_back("abh");
    indexTestFixture.search(term_list, docid_list, BWAND_OR);
    BOOST_CHECK_EQUAL(docid_list.size(), 10);


    term_list.clear();
    docid_list.clear();
    term_list.push_back("abi");
    indexTestFixture.search(term_list, docid_list, BWAND_OR);
    BOOST_CHECK_EQUAL(docid_list.size(), 9);


    term_list.clear();
    docid_list.clear();
    term_list.push_back("abp");
    indexTestFixture.search(term_list, docid_list, BWAND_AND);
    BOOST_CHECK_EQUAL(docid_list.size(), 0);
}*/


BOOST_AUTO_TEST_SUITE_END()

NS_IZENELIB_IR_END