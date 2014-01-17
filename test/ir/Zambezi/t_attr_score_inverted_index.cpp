#include "AttrScoreInvertedIndexTestFixture.h"
#include <boost/test/unit_test.hpp>
#include <util/ClockTimer.h>


NS_IZENELIB_IR_BEGIN
using namespace Zambezi;

BOOST_AUTO_TEST_SUITE(t_index_search)

BOOST_AUTO_TEST_CASE(do_search_reverse_searchBuffer_one_term)
{
    std::cout << std::endl <<"test case 1: [do_search_reverse_searchBuffer] ..." << std::endl;
    uint32_t DocNum = 30;
    AttrScoreInvertedIndexTestFixture indexTestFixture;
    bool reverse = true;
    bool search_buffer = true;
    indexTestFixture.initIndexer(DocNum, reverse, search_buffer);

    std::vector<std::string> term_list;
    std::vector<uint32_t> docid_list;
    term_list.push_back("abc");
    indexTestFixture.search(term_list, docid_list);

    BOOST_CHECK_EQUAL(docid_list.size(), 30);

    BOOST_CHECK_EQUAL(docid_list[0], 30);
    BOOST_CHECK_EQUAL(docid_list[1], 29);
    BOOST_CHECK_EQUAL(docid_list[2], 28);
    BOOST_CHECK_EQUAL(docid_list[27], 3);
    BOOST_CHECK_EQUAL(docid_list[28], 2);
    BOOST_CHECK_EQUAL(docid_list[29], 1);

    term_list.clear();
    docid_list.clear();
    
    term_list.push_back("abq");
    indexTestFixture.search(term_list, docid_list);

    BOOST_CHECK_EQUAL(docid_list.size(), 2);

    BOOST_CHECK_EQUAL(docid_list[0], 30);
    BOOST_CHECK_EQUAL(docid_list[1], 15);
}

BOOST_AUTO_TEST_CASE(do_search_forward_searchBuffer_one_term)
{
    std::cout << std::endl <<"test case 2: [do_search_forward_searchBuffer_one_term] ..." << std::endl;
    uint32_t DocNum = 30;
    AttrScoreInvertedIndexTestFixture indexTestFixture;
    bool reverse = false;
    bool search_buffer = true;
    indexTestFixture.initIndexer(DocNum, reverse, search_buffer);

    std::vector<std::string> term_list;
    std::vector<uint32_t> docid_list;
    term_list.push_back("abc");
    indexTestFixture.search(term_list, docid_list);

    BOOST_CHECK_EQUAL(docid_list.size(), 30);

    BOOST_CHECK_EQUAL(docid_list[0], 1);
    BOOST_CHECK_EQUAL(docid_list[1], 2);
    BOOST_CHECK_EQUAL(docid_list[2], 3);
    BOOST_CHECK_EQUAL(docid_list[27], 28);
    BOOST_CHECK_EQUAL(docid_list[28], 29);
    BOOST_CHECK_EQUAL(docid_list[29], 30);

    term_list.clear();
    docid_list.clear();
    
    term_list.push_back("abq");
    indexTestFixture.search(term_list, docid_list);

    BOOST_CHECK_EQUAL(docid_list.size(), 2);

    BOOST_CHECK_EQUAL(docid_list[0], 15);
    BOOST_CHECK_EQUAL(docid_list[1], 30);
}

BOOST_AUTO_TEST_CASE(do_search_reverse_searchBuffer_terms)
{
    std::cout << std::endl <<"test case 3: [do_search_reverse_searchBuffer_terms] ..." << std::endl;
    uint32_t DocNum = 30;
    AttrScoreInvertedIndexTestFixture indexTestFixture;
    bool reverse = true;
    bool search_buffer = true;
    indexTestFixture.initIndexer(DocNum, reverse, search_buffer);

    std::vector<std::string> term_list;
    std::vector<uint32_t> docid_list;
    term_list.push_back("abc");
    term_list.push_back("abd");
    indexTestFixture.search(term_list, docid_list);

    BOOST_CHECK_EQUAL(docid_list.size(), 28);

    BOOST_CHECK_EQUAL(docid_list[0], 30);
    BOOST_CHECK_EQUAL(docid_list[1], 29);
    BOOST_CHECK_EQUAL(docid_list[2], 28);

    BOOST_CHECK_EQUAL(docid_list[25], 4);
    BOOST_CHECK_EQUAL(docid_list[26], 3);
    BOOST_CHECK_EQUAL(docid_list[27], 2);
    
    term_list.clear();
    docid_list.clear();

    term_list.push_back("abc");
    term_list.push_back("abd");
    term_list.push_back("abq");
    indexTestFixture.search(term_list, docid_list);

    BOOST_CHECK_EQUAL(docid_list.size(), 2);

    BOOST_CHECK_EQUAL(docid_list[0], 30);
    BOOST_CHECK_EQUAL(docid_list[1], 15);
 }

BOOST_AUTO_TEST_CASE(do_search_forward_searchBuffer_terms)
{
    std::cout << std::endl <<"test case 4: [do_search_reverse_searchBuffer_terms] ..." << std::endl;
    uint32_t DocNum = 30;
    AttrScoreInvertedIndexTestFixture indexTestFixture;
    bool reverse = false;
    bool search_buffer = true;
    indexTestFixture.initIndexer(DocNum, reverse, search_buffer);

    std::vector<std::string> term_list;
    std::vector<uint32_t> docid_list;
    term_list.push_back("abc");
    term_list.push_back("abd");
    indexTestFixture.search(term_list, docid_list);

    BOOST_CHECK_EQUAL(docid_list.size(), 28);

    BOOST_CHECK_EQUAL(docid_list[0], 2);
    BOOST_CHECK_EQUAL(docid_list[1], 3);
    BOOST_CHECK_EQUAL(docid_list[2], 4);

    BOOST_CHECK_EQUAL(docid_list[25], 28);
    BOOST_CHECK_EQUAL(docid_list[26], 29);
    BOOST_CHECK_EQUAL(docid_list[27], 30);
    
    term_list.clear();
    docid_list.clear();
    
    term_list.push_back("abc");
    term_list.push_back("abd");
    term_list.push_back("abq");
    indexTestFixture.search(term_list, docid_list);

    BOOST_CHECK_EQUAL(docid_list.size(), 2);

    BOOST_CHECK_EQUAL(docid_list[0], 15);
    BOOST_CHECK_EQUAL(docid_list[1], 30);
}

BOOST_AUTO_TEST_CASE(do_search_reverse_searchBuffer_terms_1)
{
    std::cout << std::endl <<"test case 5: [do_search_reverse_searchBuffer_terms] more docs ..." << std::endl;
    uint32_t DocNum = 300;
    AttrScoreInvertedIndexTestFixture indexTestFixture;
    bool reverse = true;
    bool search_buffer = true;
    indexTestFixture.initIndexer(DocNum, reverse, search_buffer);

    std::vector<std::string> term_list;
    std::vector<uint32_t> docid_list;
    term_list.push_back("abc");
    term_list.push_back("abd");
    term_list.push_back("abe");
    term_list.push_back("abf");
    indexTestFixture.search(term_list, docid_list);

    BOOST_CHECK_EQUAL(docid_list.size(), 240);

    BOOST_CHECK_EQUAL(docid_list[0], 300);
    BOOST_CHECK_EQUAL(docid_list[1], 299);
    BOOST_CHECK_EQUAL(docid_list[2], 298);

    BOOST_CHECK_EQUAL(docid_list[237], 6);
    BOOST_CHECK_EQUAL(docid_list[238], 5);
    BOOST_CHECK_EQUAL(docid_list[239], 4);
}

BOOST_AUTO_TEST_CASE(do_search_reverse)
{
    std::cout << std::endl <<"test case 6: [do_search_reverse] ..." << std::endl;
    uint32_t DocNum = 300000;
    AttrScoreInvertedIndexTestFixture indexTestFixture;
    bool reverse = true;
    bool search_buffer = true;
    indexTestFixture.initIndexer(DocNum, reverse, search_buffer);

    std::vector<std::string> term_list;
    std::vector<uint32_t> docid_list;
    term_list.push_back("abc");
    term_list.push_back("abd");
    term_list.push_back("abe");
    indexTestFixture.search(term_list, docid_list);

    BOOST_CHECK_EQUAL(docid_list.size(), 260000);

    BOOST_CHECK_EQUAL(docid_list[0], 300000);
    BOOST_CHECK_EQUAL(docid_list[1], 299999);
    BOOST_CHECK_EQUAL(docid_list[2], 299998);
    BOOST_CHECK_EQUAL(docid_list[3], 299997);
    BOOST_CHECK_EQUAL(docid_list[4], 299996);
    BOOST_CHECK_EQUAL(docid_list[5], 299995);
    BOOST_CHECK_EQUAL(docid_list[6], 299994);
    BOOST_CHECK_EQUAL(docid_list[7], 299993);
    BOOST_CHECK_EQUAL(docid_list[8], 299992);
    BOOST_CHECK_EQUAL(docid_list[9], 299991);
    BOOST_CHECK_EQUAL(docid_list[10], 299990);
    BOOST_CHECK_EQUAL(docid_list[11], 299989);
    BOOST_CHECK_EQUAL(docid_list[12], 299988);
    BOOST_CHECK_EQUAL(docid_list[13], 299985); // ok
    BOOST_CHECK_EQUAL(docid_list[14], 299984); // ok
    BOOST_CHECK_EQUAL(docid_list[15], 299983); // ok

    term_list.clear();
    docid_list.clear();
    term_list.push_back("abd");
    term_list.push_back("abq");
    indexTestFixture.search(term_list, docid_list);

    BOOST_CHECK_EQUAL(docid_list.size(), 20000);

    BOOST_CHECK_EQUAL(docid_list[0], 300000);
    BOOST_CHECK_EQUAL(docid_list[1], 299985);
    BOOST_CHECK_EQUAL(docid_list[2], 299970);
    BOOST_CHECK_EQUAL(docid_list[3], 299955);
}

BOOST_AUTO_TEST_CASE(do_search_forward)
{
    std::cout << std::endl <<"test case 7: [do_search_forward] ..." << std::endl;
    uint32_t DocNum = 300000;
    AttrScoreInvertedIndexTestFixture indexTestFixture;
    bool reverse = false;
    bool search_buffer = true;
    indexTestFixture.initIndexer(DocNum, reverse, search_buffer);

    std::vector<std::string> term_list;
    std::vector<uint32_t> docid_list;
    term_list.push_back("abc");
    term_list.push_back("abd");
    term_list.push_back("abe");
    indexTestFixture.search(term_list, docid_list);

    BOOST_CHECK_EQUAL(docid_list.size(), 260000);

    BOOST_CHECK_EQUAL(docid_list[0], 3);
    BOOST_CHECK_EQUAL(docid_list[1], 4);
    BOOST_CHECK_EQUAL(docid_list[2], 5);
    BOOST_CHECK_EQUAL(docid_list[3], 6);
    BOOST_CHECK_EQUAL(docid_list[4], 7);
    BOOST_CHECK_EQUAL(docid_list[5], 8);
    BOOST_CHECK_EQUAL(docid_list[6], 9);
    BOOST_CHECK_EQUAL(docid_list[7], 10);
    BOOST_CHECK_EQUAL(docid_list[8], 11);
    BOOST_CHECK_EQUAL(docid_list[9], 12);
    BOOST_CHECK_EQUAL(docid_list[10], 13);
    BOOST_CHECK_EQUAL(docid_list[11], 14);
    BOOST_CHECK_EQUAL(docid_list[12], 15);
    BOOST_CHECK_EQUAL(docid_list[13], 18); // ok
    BOOST_CHECK_EQUAL(docid_list[14], 19); // ok
    BOOST_CHECK_EQUAL(docid_list[15], 20); // ok

    term_list.clear();
    docid_list.clear();
    term_list.push_back("abd");
    term_list.push_back("abq");
    indexTestFixture.search(term_list, docid_list);

    BOOST_CHECK_EQUAL(docid_list.size(), 20000);

    BOOST_CHECK_EQUAL(docid_list[0], 15);
    BOOST_CHECK_EQUAL(docid_list[1], 30);
    BOOST_CHECK_EQUAL(docid_list[2], 45);
    BOOST_CHECK_EQUAL(docid_list[3], 60);
}

BOOST_AUTO_TEST_CASE(do_search_new_forward)
{
    std::cout << std::endl <<"test case 8: [do_search_new_forward] ..." << std::endl;
    uint32_t DocNum = 10000000;
    AttrScoreInvertedIndexTestFixture indexTestFixture;
    bool reverse = false;
    indexTestFixture.initBIGIndexer(DocNum, reverse);

    std::vector<std::string> wordlist = indexTestFixture.getWordList();

    for (int i = 0; i < 20; ++i)
    {
        std::vector<std::string> term_list;
        std::vector<uint32_t> docid_list;
        term_list.push_back(wordlist[i]);
        term_list.push_back(wordlist[i+1]);
        indexTestFixture.search(term_list, docid_list);
        std::cout << "search: " << wordlist[i] << " / " << wordlist[i+1]
        << " :" <<docid_list.size() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(do_search_new_reverse)
{
    std::cout << std::endl <<"test case 9: [do_search_new_reverse] ..." << std::endl;
    uint32_t DocNum = 10000000;
    AttrScoreInvertedIndexTestFixture indexTestFixture;
    bool reverse = true;
    indexTestFixture.initBIGIndexer(DocNum, reverse);

    std::vector<std::string> wordlist = indexTestFixture.getWordList();
    izenelib::util::ClockTimer timer;
    for (int i = 0; i < 20; ++i)
    {
        std::vector<std::string> term_list;
        std::vector<uint32_t> docid_list;
        term_list.push_back(wordlist[i]);
        term_list.push_back(wordlist[i+1]);
        indexTestFixture.search(term_list, docid_list);
        std::cout << "search: " << wordlist[i] << " / " << wordlist[i+1]
        << " :" <<docid_list.size() << std::endl;
    }
    std::cout <<"search 20 queries cost:" << timer.elapsed() << std::endl;
}

BOOST_AUTO_TEST_CASE(do_index_save_load_reverse)
{
    std::cout << std::endl <<"test case 10: [do_index_save_load_reverse] ..." << std::endl;
    uint32_t DocNum = 2000000;
    std::vector<std::string> wordlist;
    uint32_t wordNumber = 100;
    std::vector<uint32_t> resultNumber;
    bool reverse = true;
    ///save
    {
        AttrScoreInvertedIndexTestFixture indexTestFixture;
        indexTestFixture.initBIGIndexer(DocNum, reverse);
        wordlist = indexTestFixture.getWordList();

        for (unsigned int i = 0; i < wordNumber; ++i)
        {
            std::vector<std::string> term_list;
            std::vector<uint32_t> docid_list;
            term_list.push_back(wordlist[i]);
            term_list.push_back(wordlist[i+1]);
            //term_list.push_back(wordlist[i+2]);
            indexTestFixture.search(term_list, docid_list);
            resultNumber.push_back(docid_list.size());
        }
        std::cout << "\nBegin save index ..." << std::endl;
        indexTestFixture.saveIndex("zambezi.reverse");
        std::cout << "Save index finished..." << std::endl;
    }

    /// load
    {
        AttrScoreInvertedIndexTestFixture indexTestFixture1;
        indexTestFixture1.initBIGIndexer(0, reverse);
        std::cout << "\nBegin load index ..." << std::endl;
        indexTestFixture1.loadIndex("zambezi.reverse");
        std::cout << "Load index finished..." << std::endl;

        for (unsigned int i = 0; i < wordNumber; ++i)
        {
            std::vector<std::string> term_list;
            std::vector<uint32_t> docid_list;
            term_list.push_back(wordlist[i]);
            term_list.push_back(wordlist[i+1]);
            //term_list.push_back(wordlist[i+2]);
            indexTestFixture1.search(term_list, docid_list);
            BOOST_CHECK_EQUAL(docid_list.size(), resultNumber[i]);
        }
    }
}

BOOST_AUTO_TEST_CASE(do_index_save_load_forward)
{
    std::cout << std::endl <<"test case 11: [do_index_save_load_forward] ..." << std::endl;
    uint32_t DocNum = 2000000;
    std::vector<std::string> wordlist;
    uint32_t wordNumber = 100;
    std::vector<uint32_t> resultNumber;
    bool reverse = false;
    ///save
    {
        AttrScoreInvertedIndexTestFixture indexTestFixture;
        indexTestFixture.initBIGIndexer(DocNum, reverse);
        wordlist = indexTestFixture.getWordList();

        for (unsigned int i = 0; i < wordNumber; ++i)
        {
            std::vector<std::string> term_list;
            std::vector<uint32_t> docid_list;
            term_list.push_back(wordlist[i]);
            term_list.push_back(wordlist[i+1]);
            indexTestFixture.search(term_list, docid_list);
            resultNumber.push_back(docid_list.size());
            //std::cout << "docid_list.size()" << docid_list.size() << std::endl;
        }
        std::cout << "\nBegin save index ..." << std::endl;
        indexTestFixture.saveIndex("zambezi.forward");
        std::cout << "Save index finished..." << std::endl;
    }

    /// load
    {
        AttrScoreInvertedIndexTestFixture indexTestFixture1;
        indexTestFixture1.initBIGIndexer(0, reverse);
        std::cout << "\nBegin load index ..." << std::endl;
        indexTestFixture1.loadIndex("zambezi.forward");
        std::cout << "Load index finished..." << std::endl;

        for (unsigned int i = 0; i < wordNumber; ++i)
        {
            std::vector<std::string> term_list;
            std::vector<uint32_t> docid_list;
            term_list.push_back(wordlist[i]);
            term_list.push_back(wordlist[i+1]);
            indexTestFixture1.search(term_list, docid_list);
            BOOST_CHECK_EQUAL(docid_list.size(), resultNumber[i]);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

NS_IZENELIB_IR_END
