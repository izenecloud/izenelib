#include "InvertedIndexTestFixture.h"
#include <boost/test/unit_test.hpp>
NS_IZENELIB_IR_BEGIN
using namespace Zambezi;

BOOST_AUTO_TEST_SUITE(t_index_search)

BOOST_AUTO_TEST_CASE(do_search_BWAND_AND)
{
    uint32_t DocNum = 512;
    InvertedIndexTestFixture indexTestFixture;
    indexTestFixture.initIndexer(DocNum);

    std::vector<std::string> term_list;
    std::vector<uint32_t> docid_list;
    term_list.push_back("abc");
    //term_list.push_back("abd");
    indexTestFixture.search(term_list, docid_list, BWAND_AND);
    for (std::vector<uint32_t>::iterator i = docid_list.begin(); i != docid_list.end(); ++i)
    {
        std::cout <<*i << std::endl;
    }
    BOOST_CHECK_EQUAL(docid_list.size(), 15);

/*
    term_list.clear();
    docid_list.clear();
    term_list.push_back("abd");
    indexTestFixture.search(term_list, docid_list, BWAND_AND);
    BOOST_CHECK_EQUAL(docid_list.size(), 14);

    term_list.clear();
    docid_list.clear();
    term_list.push_back("abh");
    indexTestFixture.search(term_list, docid_list, BWAND_AND);
    BOOST_CHECK_EQUAL(docid_list.size(), 10);

    term_list.clear();
    docid_list.clear();
    term_list.push_back("abi");
    indexTestFixture.search(term_list, docid_list, BWAND_AND);
    BOOST_CHECK_EQUAL(docid_list.size(), 9);


    term_list.clear();
    docid_list.clear();
    term_list.push_back("abp");
    indexTestFixture.search(term_list, docid_list, BWAND_AND);
    BOOST_CHECK_EQUAL(docid_list.size(), 0);
    */
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