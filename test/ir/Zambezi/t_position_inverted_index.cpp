#include "PositionInvertedIndexTestFixture.h"
#include <boost/test/unit_test.hpp>
#include <util/ClockTimer.h>

NS_IZENELIB_IR_BEGIN
using namespace Zambezi;

BOOST_AUTO_TEST_SUITE(t_index_search)

BOOST_AUTO_TEST_CASE(do_search_BWAND_AND_revserse)
{
    std::cout <<"test case 1: [do_search_BWAND_AND_revserse] ..." << std::endl;
    uint32_t DocNum = 300000;
    PositionInvertedIndexTestFixture indexTestFixture;
    bool reverse = true;
    indexTestFixture.initIndexer(DocNum, reverse);

    std::vector<std::string> term_list;
    std::vector<uint32_t> docid_list;
    term_list.push_back("abc");
    term_list.push_back("abd");
    term_list.push_back("abe");
    indexTestFixture.search(term_list, docid_list, BWAND_AND);

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
    BOOST_CHECK_EQUAL(docid_list[13], 299985);
    BOOST_CHECK_EQUAL(docid_list[14], 299984);
    BOOST_CHECK_EQUAL(docid_list[15], 299983);

    term_list.clear();
    docid_list.clear();
    term_list.push_back("abd");
    term_list.push_back("abq");
    indexTestFixture.search(term_list, docid_list, BWAND_AND);

    BOOST_CHECK_EQUAL(docid_list.size(), 20000);

    BOOST_CHECK_EQUAL(docid_list[0], 300000);
    BOOST_CHECK_EQUAL(docid_list[1], 299985);
    BOOST_CHECK_EQUAL(docid_list[2], 299970);
    BOOST_CHECK_EQUAL(docid_list[3], 299955);
}

BOOST_AUTO_TEST_CASE(do_search_BWAND_AND_forward)
{
    std::cout << std::endl <<"test case 2: [do_search_BWAND_AND_forward] ..." << std::endl;
    uint32_t DocNum = 300000;
    PositionInvertedIndexTestFixture indexTestFixture;
    bool reverse = false;
    indexTestFixture.initIndexer(DocNum, reverse);

    std::vector<std::string> term_list;
    std::vector<uint32_t> docid_list;
    term_list.push_back("abc");
    term_list.push_back("abd");
    term_list.push_back("abe");
    indexTestFixture.search(term_list, docid_list, BWAND_AND);

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
    indexTestFixture.search(term_list, docid_list, BWAND_AND);

    BOOST_CHECK_EQUAL(docid_list.size(), 20000);

    BOOST_CHECK_EQUAL(docid_list[0], 15);
    BOOST_CHECK_EQUAL(docid_list[1], 30);
    BOOST_CHECK_EQUAL(docid_list[2], 45);
    BOOST_CHECK_EQUAL(docid_list[3], 60);
}

// BOOST_AUTO_TEST_CASE(do_search_BWAND_OR_revserse)
// {
//     std::cout <<"test case 3: [do_search_BWAND_OR_revserse] ..." << std::endl;
//     uint32_t DocNum = 300000;
//     PositionInvertedIndexTestFixture indexTestFixture;
//     bool reverse = true;
//     indexTestFixture.initIndexer(DocNum, reverse);

//     std::vector<std::string> term_list;
//     std::vector<uint32_t> docid_list;
//     term_list.push_back("abc");
//     term_list.push_back("abd");
//     term_list.push_back("abe");
//     indexTestFixture.search(term_list, docid_list, BWAND_OR);

//     BOOST_CHECK_EQUAL(docid_list.size(), 300000);

//     BOOST_CHECK_EQUAL(docid_list[0], 1);
//     BOOST_CHECK_EQUAL(docid_list[299999], 300000);

//     term_list.clear();
//     docid_list.clear();
//     term_list.push_back("abd");
//     term_list.push_back("abq");
//     indexTestFixture.search(term_list, docid_list, BWAND_OR);

//     BOOST_CHECK_EQUAL(docid_list.size(), 20000);

//     term_list.clear();
//     docid_list.clear();
//     term_list.push_back("abd");
//     term_list.push_back("abq");
//     indexTestFixture.search(term_list, docid_list, BWAND_OR);

//     BOOST_CHECK_EQUAL(docid_list.size(), 280000);

//     term_list.clear();
//     docid_list.clear();
//     term_list.push_back("abe");
//     term_list.push_back("abq");
//     indexTestFixture.search(term_list, docid_list, BWAND_OR);

//     BOOST_CHECK_EQUAL(docid_list.size(), 260000);
// }

// BOOST_AUTO_TEST_CASE(do_search_BWAND_OR_forward)
// {
//     std::cout << std::endl <<"test case 4: [do_search_BWAND_OR_forward] ..." << std::endl;
//     uint32_t DocNum = 300000;
//     PositionInvertedIndexTestFixture indexTestFixture;
//     bool reverse = false;
//     indexTestFixture.initIndexer(DocNum, reverse);

//     std::vector<std::string> term_list;
//     std::vector<uint32_t> docid_list;
//     term_list.push_back("abc");
//     term_list.push_back("abd");
//     term_list.push_back("abe");
//     indexTestFixture.search(term_list, docid_list, BWAND_OR);

//     BOOST_CHECK_EQUAL(docid_list.size(), 300000);

//     BOOST_CHECK_EQUAL(docid_list[0], 1);
//     BOOST_CHECK_EQUAL(docid_list[299999], 300000);

//     term_list.clear();
//     docid_list.clear();
//     term_list.push_back("abd");
//     term_list.push_back("abq");
//     indexTestFixture.search(term_list, docid_list, BWAND_OR);

//     BOOST_CHECK_EQUAL(docid_list.size(), 20000);

//     term_list.clear();
//     docid_list.clear();
//     term_list.push_back("abd");
//     term_list.push_back("abq");
//     indexTestFixture.search(term_list, docid_list, BWAND_OR);

//     BOOST_CHECK_EQUAL(docid_list.size(), 280000);

//     term_list.clear();
//     docid_list.clear();
//     term_list.push_back("abe");
//     term_list.push_back("abq");
//     indexTestFixture.search(term_list, docid_list, BWAND_OR);

//     BOOST_CHECK_EQUAL(docid_list.size(), 260000);
    
// }

BOOST_AUTO_TEST_CASE(do_search_SVS_forward)
{
    std::cout << std::endl <<"test case 5: [do_search_SVS_forward] ..." << std::endl;
    uint32_t DocNum = 10000000;
    PositionInvertedIndexTestFixture indexTestFixture;
    bool reverse = false;
    indexTestFixture.initBIGIndexer(DocNum, reverse);

    std::vector<std::string> wordlist = indexTestFixture.getWordList();

    for (int i = 0; i < 100; ++i)
    {
        std::vector<std::string> term_list;
        std::vector<uint32_t> docid_list;
        term_list.push_back(wordlist[i]);
        term_list.push_back(wordlist[i+1]);
        term_list.push_back(wordlist[i+2]);
        indexTestFixture.search(term_list, docid_list, SVS);
        std::cout << "search: " << wordlist[i] << " / " << wordlist[i+1] << " / " << wordlist[i+2]
        << " :" <<docid_list.size() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(do_search_SVS_reverse)
{
    std::cout << std::endl <<"test case 6: [do_search_SVS_reverse] ..." << std::endl;
    uint32_t DocNum = 10000000;
    PositionInvertedIndexTestFixture indexTestFixture;
    bool reverse = true;
    indexTestFixture.initBIGIndexer(DocNum, reverse);

    std::vector<std::string> wordlist = indexTestFixture.getWordList();

    izenelib::util::ClockTimer timer;
    for (int i = 0; i < 100; ++i)
    {
        std::vector<std::string> term_list;
        std::vector<uint32_t> docid_list;
        term_list.push_back(wordlist[i]);
        term_list.push_back(wordlist[i+1]);
        term_list.push_back(wordlist[i+2]);
        indexTestFixture.search(term_list, docid_list, SVS);
        std::cout << "search: " << wordlist[i] << " / " << wordlist[i+1] << " / " << wordlist[i+2]
        << " :" <<docid_list.size() << std::endl;
    }
    std::cout <<"search 100 queries cost:" << timer.elapsed() << std::endl;
}

BOOST_AUTO_TEST_CASE(do_index_save_load_reverse_SVS)
{
    std::cout << std::endl <<"test case 7: [do_index_save_load_reverse_SVS] ..." << std::endl;
    uint32_t DocNum = 2000000;
    std::vector<std::string> wordlist;
    uint32_t wordNumber = 100;
    std::vector<uint32_t> resultNumber;
    bool reverse = true;
    ///save
    {
        PositionInvertedIndexTestFixture indexTestFixture;
        indexTestFixture.initBIGIndexer(DocNum, reverse);
        wordlist = indexTestFixture.getWordList();

        for (unsigned int i = 0; i < wordNumber; ++i)
        {
            std::vector<std::string> term_list;
            std::vector<uint32_t> docid_list;
            term_list.push_back(wordlist[i]);
            term_list.push_back(wordlist[i+1]);
            //term_list.push_back(wordlist[i+2]);
            indexTestFixture.search(term_list, docid_list, SVS);
            resultNumber.push_back(docid_list.size());
        }
        std::cout << "\nBegin save index ..." << std::endl;
        indexTestFixture.saveIndex("Position_zambezi.reverse");
        std::cout << "Save index finished..." << std::endl;
    }

    /// load
    {
        PositionInvertedIndexTestFixture indexTestFixture1;
        indexTestFixture1.initBIGIndexer(0, reverse);
        std::cout << "\nBegin load index ..." << std::endl;
        indexTestFixture1.loadIndex("Position_zambezi.reverse");
        std::cout << "Load index finished..." << std::endl;

        for (unsigned int i = 0; i < wordNumber; ++i)
        {
            std::vector<std::string> term_list;
            std::vector<uint32_t> docid_list;
            term_list.push_back(wordlist[i]);
            term_list.push_back(wordlist[i+1]);
            //term_list.push_back(wordlist[i+2]);
            indexTestFixture1.search(term_list, docid_list, SVS);
            BOOST_CHECK_EQUAL(docid_list.size(), resultNumber[i]);
        }
    }
}

BOOST_AUTO_TEST_CASE(do_index_save_load_forward_SVS)
{
    std::cout << std::endl <<"test case 8: [do_index_save_load_forward_SVS] ..." << std::endl;
    uint32_t DocNum = 2000000;
    std::vector<std::string> wordlist;
    uint32_t wordNumber = 100;
    std::vector<uint32_t> resultNumber;
    bool reverse = false;
    ///save
    {
        PositionInvertedIndexTestFixture indexTestFixture;
        indexTestFixture.initBIGIndexer(DocNum, reverse);
        wordlist = indexTestFixture.getWordList();

        for (unsigned int i = 0; i < wordNumber; ++i)
        {
            std::vector<std::string> term_list;
            std::vector<uint32_t> docid_list;
            term_list.push_back(wordlist[i]);
            term_list.push_back(wordlist[i+1]);
            indexTestFixture.search(term_list, docid_list, SVS);
            resultNumber.push_back(docid_list.size());
            //std::cout << "docid_list.size()" << docid_list.size() << std::endl;
        }
        std::cout << "\nBegin save index ..." << std::endl;
        indexTestFixture.saveIndex("Position_zambezi.forward");
        std::cout << "Save index finished..." << std::endl;
    }

    /// load
    {
        PositionInvertedIndexTestFixture indexTestFixture1;
        indexTestFixture1.initBIGIndexer(0, reverse);
        std::cout << "\nBegin load index ..." << std::endl;
        indexTestFixture1.loadIndex("Position_zambezi.forward");
        std::cout << "Load index finished..." << std::endl;

        for (unsigned int i = 0; i < wordNumber; ++i)
        {
            std::vector<std::string> term_list;
            std::vector<uint32_t> docid_list;
            term_list.push_back(wordlist[i]);
            term_list.push_back(wordlist[i+1]);
            indexTestFixture1.search(term_list, docid_list, SVS);
            BOOST_CHECK_EQUAL(docid_list.size(), resultNumber[i]);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

NS_IZENELIB_IR_END
