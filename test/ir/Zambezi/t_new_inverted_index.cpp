#include "newInvertedIndexTestFixture.h"
#include <boost/test/unit_test.hpp>
NS_IZENELIB_IR_BEGIN
using namespace Zambezi;

BOOST_AUTO_TEST_SUITE(t_index_search)

BOOST_AUTO_TEST_CASE(do_search_reverse)
{
    std::cout <<"test case 1: [do_search_reverse] ..." << std::endl;
    uint32_t DocNum = 300000;
    newInvertedIndexTestFixture indexTestFixture;
    bool reverse = true;
    indexTestFixture.initIndexer(DocNum, reverse);

    std::vector<std::string> term_list;
    std::vector<uint32_t> docid_list;
    term_list.push_back("abc");
    term_list.push_back("abd");
    term_list.push_back("abe");
    indexTestFixture.search(term_list, docid_list);
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
    indexTestFixture.search(term_list, docid_list);
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

BOOST_AUTO_TEST_CASE(do_search_forward)
{
    std::cout << std::endl <<"test case 2: [do_search_forward] ..." << std::endl;
    uint32_t DocNum = 300000;
    newInvertedIndexTestFixture indexTestFixture;
    bool reverse = false;
    indexTestFixture.initIndexer(DocNum, reverse);

    std::vector<std::string> term_list;
    std::vector<uint32_t> docid_list;
    term_list.push_back("abc");
    term_list.push_back("abd");
    term_list.push_back("abe");
    indexTestFixture.search(term_list, docid_list);
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
    indexTestFixture.search(term_list, docid_list);
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

BOOST_AUTO_TEST_CASE(do_search_new_forward)
{
    std::cout << std::endl <<"test case 3: [do_search_new_forward] ..." << std::endl;
    uint32_t DocNum = 10000000;
    newInvertedIndexTestFixture indexTestFixture;
    bool reverse = false;
    indexTestFixture.initBIGIndexer(DocNum, reverse);

    std::vector<std::string> wordlist = indexTestFixture.getWordList();

    for (int i = 0; i < 20; ++i)
    {
        std::vector<std::string> term_list;
        std::vector<uint32_t> docid_list;
        term_list.push_back(wordlist[i]);
        term_list.push_back(wordlist[i+1]);
        term_list.push_back(wordlist[i+2]);
        term_list.push_back(wordlist[i+3]);
        indexTestFixture.search(term_list, docid_list);
        std::cout << "search: " << wordlist[i] << " / " << wordlist[i+1] << " / " << wordlist[i+2] 
        << " / " << wordlist[i+3] << " :" <<docid_list.size() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(do_search_new_reverse)
{
    std::cout << std::endl <<"test case 4: [do_search_new_reverse] ..." << std::endl;
    uint32_t DocNum = 10000000;
    newInvertedIndexTestFixture indexTestFixture;
    bool reverse = true;
    indexTestFixture.initBIGIndexer(DocNum, reverse);

    std::vector<std::string> wordlist = indexTestFixture.getWordList();

    for (int i = 0; i < 20; ++i)
    {
        std::vector<std::string> term_list;
        std::vector<uint32_t> docid_list;
        term_list.push_back(wordlist[i]);
        term_list.push_back(wordlist[i+1]);
        term_list.push_back(wordlist[i+2]);
        term_list.push_back(wordlist[i+3]);
        indexTestFixture.search(term_list, docid_list);
        std::cout << "search: " << wordlist[i] << " / " << wordlist[i+1] << " / " << wordlist[i+2] 
        << " / " << wordlist[i+3] << " :" <<docid_list.size() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(do_index_save_load_reverse)
{
    std::cout << std::endl <<"test case 5: [do_index_save_load_reverse] ..." << std::endl;
    uint32_t DocNum = 2000000;
    std::vector<std::string> wordlist;
    uint32_t wordNumber = 50;
    std::vector<uint32_t> resultNumber;
    bool reverse = true;
    ///save
    {
      newInvertedIndexTestFixture indexTestFixture;
      indexTestFixture.initBIGIndexer(DocNum, reverse);
      wordlist = indexTestFixture.getWordList();

      for (unsigned int i = 0; i < wordNumber; ++i)
      {
          std::vector<std::string> term_list;
          std::vector<uint32_t> docid_list;
          term_list.push_back(wordlist[i]);
          term_list.push_back(wordlist[i+1]);
          term_list.push_back(wordlist[i+2]);
          indexTestFixture.search(term_list, docid_list);
          resultNumber.push_back(docid_list.size());
      }
      std::cout << indexTestFixture.getTotalDocCount() << std::endl;
      std::cout <<"Begin save index ..." ;
      indexTestFixture.saveIndex();
      std::cout <<"   Save index finished..." << std::endl;
    }

    /// load
    {
      newInvertedIndexTestFixture indexTestFixture1;
      indexTestFixture1.initBIGIndexer(0, reverse);
      std::cout <<"begin load index ..." ;
      indexTestFixture1.loadIndex();
      std::cout <<"   Load index finished..." << std::endl;
      std::cout << indexTestFixture1.getTotalDocCount() << std::endl;

      for (unsigned int i = 0; i < wordNumber; ++i)
      {
          std::vector<std::string> term_list;
          std::vector<uint32_t> docid_list;
          term_list.push_back(wordlist[i]);
          term_list.push_back(wordlist[i+1]);
          term_list.push_back(wordlist[i+2]);
          indexTestFixture1.search(term_list, docid_list);
          BOOST_CHECK_EQUAL(docid_list.size(), resultNumber[i]);
      }
    }
}

BOOST_AUTO_TEST_CASE(do_index_save_load_forward)
{
    std::cout << std::endl <<"test case 6: [do_index_save_load_forward] ..." << std::endl;
    uint32_t DocNum = 2000000;
    std::vector<std::string> wordlist;
    uint32_t wordNumber = 50;
    std::vector<uint32_t> resultNumber;
    bool reverse = false;
    ///save
    {
      newInvertedIndexTestFixture indexTestFixture;
      indexTestFixture.initBIGIndexer(DocNum, reverse);
      wordlist = indexTestFixture.getWordList();

      for (unsigned int i = 0; i < wordNumber; ++i)
      {
          std::vector<std::string> term_list;
          std::vector<uint32_t> docid_list;
          term_list.push_back(wordlist[i]);
          term_list.push_back(wordlist[i+1]);
          term_list.push_back(wordlist[i+2]);
          indexTestFixture.search(term_list, docid_list);
          resultNumber.push_back(docid_list.size());
      }
      std::cout << indexTestFixture.getTotalDocCount() << std::endl;
      std::cout <<"Begin save index ..." ;
      indexTestFixture.saveIndex();
      std::cout <<"   Save index finished..." << std::endl;
    }

    /// load
    {
      newInvertedIndexTestFixture indexTestFixture1;
      indexTestFixture1.initBIGIndexer(0, reverse);
      std::cout <<"begin load index ..." ;
      indexTestFixture1.loadIndex();
      std::cout <<"   Load index finished..." << std::endl;
      std::cout << indexTestFixture1.getTotalDocCount() << std::endl;

      for (unsigned int i = 0; i < wordNumber; ++i)
      {
          std::vector<std::string> term_list;
          std::vector<uint32_t> docid_list;
          term_list.push_back(wordlist[i]);
          term_list.push_back(wordlist[i+1]);
          term_list.push_back(wordlist[i+2]);
          indexTestFixture1.search(term_list, docid_list);
          BOOST_CHECK_EQUAL(docid_list.size(), resultNumber[i]);
      }
    }
}

BOOST_AUTO_TEST_SUITE_END()

NS_IZENELIB_IR_END