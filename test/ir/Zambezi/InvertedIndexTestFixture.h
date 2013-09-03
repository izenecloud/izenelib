/**
* @file       InvertedIndexTestFixture.h
* @author     Hongliang
* @version    1.0
* @brief Fixture to test Inverted Index module.
*
*/

#ifndef INVERTED_INDEX_TEST_FIXTURE_H
#define INVERTED_INDEX_TEST_FIXTURE_H

#include <string>
#include <vector>
#include <map>
#include <glog/logging.h>
#include <iostream>
#include <stdlib.h>
#include <ir/Zambezi/InvertedIndex.hpp>
NS_IZENELIB_IR_BEGIN

namespace Zambezi
{
    typedef std::vector<std::string> DocIdListT;
    typedef std::map<uint32_t, DocIdListT> DocIDTermMapT;

    class InvertedIndexTestFixture
    {
    public:
        InvertedIndexTestFixture()
            :index_(NULL)
        {
        }

        void initIndex(bool isReverse)
        {
            index_ = new InvertedIndex(NON_POSITIONAL, isReverse);
        }

        ~InvertedIndexTestFixture()
        {
            delete index_;
        }

        void prepareDocument(DocIDTermMapT& DocIdTermMap, uint32_t docNumber)
        {
            std::vector<uint32_t> DocIdList;
            for (unsigned int i = 1; i <= docNumber; ++i)
            {
                DocIdList.push_back(i);
            }

            std::vector<std::string> termList;
            std::vector<std::string> term_tmp;
            term_tmp.push_back("abc");
            term_tmp.push_back("abd");
            term_tmp.push_back("abe");
            term_tmp.push_back("abf");
            term_tmp.push_back("abg");
            term_tmp.push_back("abh");
            term_tmp.push_back("abi");
            term_tmp.push_back("abj");
            term_tmp.push_back("abk");
            term_tmp.push_back("abl");
            term_tmp.push_back("abm");
            term_tmp.push_back("abn");
            term_tmp.push_back("abo");
            term_tmp.push_back("abp");
            term_tmp.push_back("abq");
            unsigned int x = 0;
            for (std::vector<uint32_t>::iterator i = DocIdList.begin(); i != DocIdList.end(); ++i)
            {
                termList.push_back(term_tmp[x]);
                DocIdTermMap[*i] = termList;
                x++;
                if (x == term_tmp.size() )
                {
                    termList.clear();
                    x = 0;
                }
            }
        }

        void prepareBigDocument(DocIDTermMapT& DocIdTermMap, uint32_t docNumber)
        {
            //10w key word; 5-10;
            //10M document;
            //each document; 10-15words;

            std::vector<std::string> charString; /// number:37
            charString.push_back("a");
            charString.push_back("b");
            charString.push_back("c");
            charString.push_back("d");
            charString.push_back("e");
            charString.push_back("f");
            charString.push_back("g");
            charString.push_back("h");
            charString.push_back("i");
            charString.push_back("j");
            charString.push_back("k");
            charString.push_back("l");
            charString.push_back("m");
            charString.push_back("n");
            charString.push_back("o");
            charString.push_back("p");
            charString.push_back("q");
            charString.push_back("r");
            charString.push_back("s");
            charString.push_back("t");
            charString.push_back("u");
            charString.push_back("v");
            charString.push_back("w");
            charString.push_back("x");
            charString.push_back("y");
            charString.push_back("z");
            charString.push_back("1");
            charString.push_back("2");
            charString.push_back("3");
            charString.push_back("4");
            charString.push_back("5");
            charString.push_back("6");
            charString.push_back("7");
            charString.push_back("8");
            charString.push_back("9");
            charString.push_back("0");
            charString.push_back("-");
            charString.push_back("鞋");
            charString.push_back("衣");
            charString.push_back("裙");
            charString.push_back("裤");
            charString.push_back("运");
            charString.push_back("动");
            int termNumber = 80000;

            std::string freq_word = "123abc";
            //build word;
            srand( (unsigned int)time(0) );
            for (int i = 0; i < termNumber; ++i)
            {
                int wordlen = rand()%5 + 2;
                std::string newword;
                for (int i = 0; i < wordlen; ++i)
                {
                    int randchar = rand()%charString.size();
                    newword += charString[randchar];
                }
                wordlist_.push_back(newword);
                std::cout <<"term:" << newword << endl;
            }

            //build term;
            std::vector<uint32_t> DocIdList;
            for (int i = 1; i <= docNumber; ++i)
            {
                DocIdList.push_back(i);
            }
            for (std::vector<uint32_t>::iterator i = DocIdList.begin(); i != DocIdList.end(); ++i)
            {
                std::vector<std::string> termList;
                int wordNumber = rand()%4 + 5;
                for (int j = 0; j < wordNumber; ++j)
                {
                    int randwrod = rand()%wordlist_.size();
                    termList.push_back(wordlist_[randwrod]);
                }
                termList.push_back(freq_word);
                DocIdTermMap[*i] = termList;
            }
            int x;
            cin>>x;
        }

        void initBIGIndexer(uint32_t docNumber, bool reverse)
        {
            DocIDTermMapT docTermMap;

            prepareBigDocument(docTermMap, docNumber);

            initIndex(reverse);

            buildIndex(docTermMap);
        }

        void initIndexer(uint32_t docNumber, bool reverse)
        {
            DocIDTermMapT docTermMap;

            prepareDocument(docTermMap, docNumber);

            initIndex(reverse);

            buildIndex(docTermMap);
        }

        void buildIndex(const DocIDTermMapT& docTermMap)
        {
            int count = 0;
            for (DocIDTermMapT::const_iterator i = docTermMap.begin(); i != docTermMap.end(); ++i)
            {
                index_->insertDoc(i->first, i->second);
                if (count%1000000 == 0)
                {
                    LOG(INFO) << "insert document:" << count;
                }
                count++;
            }
            index_->flush();
        }

        void search(const std::vector<std::string>& term_list, std::vector<uint32_t>& docid_list, Algorithm algorithm)
        {
            std::vector<float> score_list;
            uint32_t hits = 10000000;
            index_->retrieval(
                algorithm,
                term_list,
                hits,
                docid_list,
                score_list);
        }

        std::vector<std::string>& getWordList()
        {
            return wordlist_;
        }

    private:
        InvertedIndex* index_;
        std::string indePath_;
        std::vector<std::string> wordlist_;
    };
}

NS_IZENELIB_IR_END

#endif ///INVERTED_INDEX_TEST_FIXTURE_H
